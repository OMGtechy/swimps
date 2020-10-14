#include "swimps-time.h"
#include "swimps-log.h"
#include "swimps-trace-file.h"
#include "swimps-io.h"

#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <cstring>

#include <unistd.h>
#include <signal.h>
#include <execinfo.h>

namespace {

    constexpr clockid_t clockID = CLOCK_MONOTONIC;

    std::atomic_flag sigprofRunningFlag = ATOMIC_FLAG_INIT;
    int traceFile = -1;
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
                const char message[] = "swimps::time::now failed whilst taking sample.";
                swimps::log::write_to_log(
                    swimps::log::LogLevel::Error,
                    message,
                    sizeof message
                );

                goto swimps_preload_sigprof_cleanup;
            }
        }

        swimps_trace_file_add_sample(traceFile, &sample);

        {
            void* backtraceBuffer[2048];
            const int numberOfStackFrames = backtrace(backtraceBuffer, sizeof backtraceBuffer / sizeof backtraceBuffer[0]);

            swimps_trace_file_add_raw_backtrace(traceFile,
                                                sample.backtraceID,
                                                backtraceBuffer,
                                                numberOfStackFrames);
        }

    swimps_preload_sigprof_cleanup:
        sigprofRunningFlag.clear();
    }

    int swimps_preload_create_trace_file() {
        swimps::time::TimeSpecification time;
        if (swimps::time::now(clockID, time) == -1) {

            const char message[] = "Could not get time to generate trace file name.";
            swimps::log::write_to_log(
                swimps::log::LogLevel::Fatal,
                message,
                sizeof message
            );

            abort();
        }

        char traceFileNameBuffer[2048] = { 0 };

        const size_t bytesWritten = swimps_trace_file_generate_name(
            program_invocation_short_name,
            &time,
            getpid(),
            traceFileNameBuffer,
            sizeof traceFileNameBuffer
        );

        // Whilst it could be *exactly* the right size,
        // chances are there's just not enough room.
        if (bytesWritten == sizeof traceFileNameBuffer) {

            const char message[] = "Could not generate trace file name.";
            swimps::log::write_to_log(
                swimps::log::LogLevel::Fatal,
                message,
                sizeof message
            );

            abort();
        }

        const int file = swimps_trace_file_create(traceFileNameBuffer);
        if (file == -1) {

            const char message[] = "Could not create trace file.";
            swimps::log::write_to_log(
                swimps::log::LogLevel::Fatal,
                message,
                sizeof message
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

    __attribute__((constructor))
    void swimps_preload_constructor() {
        // From https://man7.org/linux/man-pages/man3/backtrace.3.html:
        //
        // "backtrace() and backtrace_symbols_fd() don't call malloc()
        //  explicitly, but they are part of libgcc, which gets loaded
        //  dynamically when first used.  Dynamic loading usually triggers a
        //  call to malloc(3).  If you need certain calls to these two
        //  functions to not allocate memory (in signal handlers, for
        //  example), you need to make sure libgcc is loaded beforehand."
        //
        // TL;DR: Force libgcc to load so backtrace and backtrace_symbols_fd functions are signal safe.
        {
            void* dummy[1];
            backtrace(dummy, 1);
        }

        traceFile = swimps_preload_create_trace_file();

        if (swimps_preload_setup_signal_handler() == -1) {
            const char formatBuffer[] = "Could not setup signal handler, errno %d (%s).";

            swimps::log::format_and_write_to_log<1024>(
                swimps::log::LogLevel::Fatal,
                formatBuffer,
                sizeof formatBuffer,
                errno,
                strerror(errno)
            );

            abort();
        }

        if (swimps::time::create_signal_timer(clockID, SIGPROF, sampleTimer) == -1) {
            const char formatBuffer[] = "Could not create timer, errno %d (%s).";

            swimps::log::format_and_write_to_log<1024>(
                swimps::log::LogLevel::Fatal,
                formatBuffer,
                sizeof formatBuffer,
                errno,
                strerror(errno)
            );

            abort();
        }

        if (swimps_preload_start_timer(sampleTimer) == -1) {
            const char formatBuffer[] = "Could not start timer, errno %d (%s).";

            swimps::log::format_and_write_to_log<1024>(
                swimps::log::LogLevel::Fatal,
                formatBuffer,
                sizeof formatBuffer,
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
        swimps_trace_file_finalise(traceFile);
    }

}
