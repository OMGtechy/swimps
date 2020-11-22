#include "swimps-time.h"
#include "swimps-log.h"
#include "swimps-trace-file.h"
#include "swimps-io.h"
#include "swimps-preload.h"
#include "swimps-option.h"

#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <cstring>

#include <unistd.h>
#include <signal.h>
#include <limits.h>

namespace {

    constexpr clockid_t clockID = CLOCK_MONOTONIC;

    std::atomic_flag sigprofRunningFlag = ATOMIC_FLAG_INIT;
    int traceFile = -1;
    char traceFilePath[PATH_MAX] = { };
    timer_t sampleTimer;
    swimps::trace::backtrace_id_t nextBacktraceID = 1;

    void swimps_preload_sigprof_handler(const int) {
        if (sigprofRunningFlag.test_and_set()) {
            // Drop samples that occur when a sample is already being taken.
            return;
        }

        swimps::trace::Sample sample;
        sample.backtraceID = nextBacktraceID++;

        {
            if (swimps::time::now(clockID, sample.timestamp) != 0) {
                swimps::log::write_to_log(
                    swimps::log::LogLevel::Error,
                    "swimps::time::now failed whilst taking sample."
                );

                goto swimps_preload_sigprof_cleanup;
            }
        }

        swimps::trace::file::add_sample(traceFile, sample);

        {
            auto backtrace = swimps::preload::get_backtrace();
            backtrace.id = sample.backtraceID;
            swimps::trace::file::add_backtrace(
                traceFile,
                backtrace
            );
        }

    swimps_preload_sigprof_cleanup:
        sigprofRunningFlag.clear();
    }

    int swimps_preload_create_trace_file(char* traceFilePath, size_t traceFilePathSize) {
        swimps::time::TimeSpecification time;
        if (swimps::time::now(clockID, time) == -1) {
            swimps::log::write_to_log(
                swimps::log::LogLevel::Fatal,
                "Could not get time to generate trace file name."
            );

            abort();
        }

        getcwd(traceFilePath, traceFilePathSize);
        size_t bytesWritten = strnlen(traceFilePath, traceFilePathSize);
        traceFilePath += bytesWritten;
        traceFilePathSize -= bytesWritten;

        if (traceFilePathSize == 0) {
            swimps::log::write_to_log(
                swimps::log::LogLevel::Fatal,
                "Ran out of space when generating trace file name."
            );

            abort();
        }

        // replace null terminator with /
        *traceFilePath = '/';
        traceFilePath += 1;
        bytesWritten += 1;

        bytesWritten += swimps::trace::file::generate_name(
            program_invocation_short_name,
            time,
            getpid(),
            traceFilePath,
            traceFilePathSize
        );

        // Whilst it could be *exactly* the right size,
        // chances are there's just not enough room.
        if (bytesWritten == traceFilePathSize) {
            swimps::log::write_to_log(
                swimps::log::LogLevel::Fatal,
                "Could not generate trace file name."
            );

            abort();
        }

        const int file = swimps::trace::file::create(traceFilePath);
        if (file == -1) {
            swimps::log::write_to_log(
                swimps::log::LogLevel::Fatal,
                "Could not create trace file."
            );

            abort();
        }

        return file;
    }

    int swimps_preload_setup_signal_handler() {
        struct sigaction action;
        action.sa_handler = swimps_preload_sigprof_handler;
        action.sa_flags = SA_SIGINFO | SA_RESTART;
        sigemptyset(&action.sa_mask);

        return sigaction(SIGPROF, &action, NULL);
    }

    int swimps_preload_start_timer(timer_t timer) {
        struct itimerspec timerSpec;
        timerSpec.it_interval.tv_sec = 1;
        timerSpec.it_interval.tv_nsec = 0;
        timerSpec.it_value = timerSpec.it_interval;
        return timer_settime(timer, 0, &timerSpec, NULL);
    }

    int swimps_preload_stop_timer(timer_t timer) {
        struct itimerspec timerSpec;
        timerSpec.it_interval.tv_sec = 0;
        timerSpec.it_interval.tv_nsec = 0;
        timerSpec.it_value = timerSpec.it_interval;
        return timer_settime(timer, 0, &timerSpec, NULL);
    }

    void load_options() {
        const auto options = swimps::option::Options::fromString(std::getenv("SWIMPS_OPTIONS"));
        swimps::log::setLevelToLog(options.logLevel);
    }

    __attribute__((constructor))
    void swimps_preload_constructor() {
        load_options();

        traceFile = swimps_preload_create_trace_file(traceFilePath, sizeof traceFilePath);

        if (swimps_preload_setup_signal_handler() == -1) {
            swimps::log::format_and_write_to_log<1024>(
                swimps::log::LogLevel::Fatal,
                "Could not setup signal handler, errno %d (%s).",
                errno,
                strerror(errno)
            );

            abort();
        }

        if (swimps::time::create_signal_timer(clockID, SIGPROF, sampleTimer) == -1) {
            swimps::log::format_and_write_to_log<1024>(
                swimps::log::LogLevel::Fatal,
                "Could not create timer, errno %d (%s).",
                errno,
                strerror(errno)
            );

            abort();
        }

        // Everything from here onwards must be signal safe.

        if (swimps_preload_start_timer(sampleTimer) == -1) {
            swimps::log::format_and_write_to_log<1024>(
                swimps::log::LogLevel::Fatal,
                "Could not start timer, errno %d (%s).",
                errno,
                strerror(errno)
            );

            abort();
        }
    }

    __attribute__((destructor))
    void swimps_preload_destructor() {
        // Disable the signal handler so that non-async signal safe code can be used.
        // TODO: is this safe if the timer fails to get created properly?
        swimps_preload_stop_timer(sampleTimer);

        // Wait until the all in-progress samples are finished.
        while (sigprofRunningFlag.test_and_set());

        // Tidy up the data in the trace file.
        // TODO: What happens if the trace file fails to be created properly?
        if (swimps::trace::file::finalise(traceFile, traceFilePath, strnlen(traceFilePath, sizeof traceFilePath)) != 0) {
            abort();
        }
    }
}
