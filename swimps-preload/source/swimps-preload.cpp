#include "swimps-time.h"
#include "swimps-log.h"
#include "swimps-trace-file.h"
#include "swimps-io.h"
#include "swimps-preload.h"
#include "swimps-option-options.h"

#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include <unistd.h>
#include <signal.h>
#include <limits.h>

using swimps::io::File;
using swimps::io::write_to_buffer;
using swimps::log::LogLevel;
using swimps::log::write_to_log;
using swimps::preload::get_proc_maps;
using swimps::option::Options;
using swimps::trace::TraceFile;

namespace {

    constexpr clockid_t clockID = CLOCK_MONOTONIC;

    std::atomic_flag sigprofRunningFlag = ATOMIC_FLAG_INIT;
    TraceFile traceFile;
    std::array<char, PATH_MAX> targetProgram = { };
    std::array<char, PATH_MAX> traceFilePath = { };
    timer_t sampleTimer;
    swimps::trace::backtrace_id_t nextBacktraceID = 1;
    swimps::trace::stack_frame_id_t nextStackFrameID = 1;

    void swimps_preload_sigprof_handler(const int, siginfo_t*, void* context) {
        if (sigprofRunningFlag.test_and_set()) {
            // Drop samples that occur when a sample is already being taken.
            return;
        }

        swimps::trace::Sample sample;
        sample.backtraceID = nextBacktraceID;

        {
            if (swimps::time::now(clockID, sample.timestamp) != 0) {
                swimps::log::write_to_log(
                    swimps::log::LogLevel::Error,
                    "swimps::time::now failed whilst taking sample."
                );

                goto swimps_preload_sigprof_cleanup;
            }
        }

        traceFile.add_sample(sample);

        {
            const auto result = swimps::preload::get_backtrace(
                static_cast<ucontext_t*>(context),
                nextBacktraceID,
                nextStackFrameID
            );

            const auto& backtrace = std::get<0>(result);
            const auto& stackFrames = std::get<1>(result);

            for (swimps::trace::stack_frame_count_t i = 0; i < backtrace.stackFrameIDCount; ++i) {
                traceFile.add_stack_frame(stackFrames[i]);
            }

            traceFile.add_backtrace(backtrace);
        }

    swimps_preload_sigprof_cleanup:
        sigprofRunningFlag.clear();
    }

    template <std::size_t TraceFilePathSize>
    TraceFile swimps_preload_create_trace_file(std::array<char, TraceFilePathSize>& traceFilePath, const Options& options) {
        const size_t pathLength = std::min(options.targetTraceFile.size(), TraceFilePathSize);
        swimps::io::write_to_buffer({ options.targetTraceFile.c_str(), options.targetTraceFile.size() }, traceFilePath);
        return TraceFile::create({ traceFilePath.data(), pathLength }, TraceFile::Permissions::ReadWrite);
    }

    int swimps_preload_setup_signal_handler() {
        struct sigaction action;
        action.sa_sigaction = swimps_preload_sigprof_handler;
        action.sa_flags = SA_SIGINFO | SA_RESTART;
        sigemptyset(&action.sa_mask);

        return sigaction(SIGPROF, &action, NULL);
    }

    int swimps_preload_start_timer(timer_t timer, const double samplesPerSecond) {
        struct itimerspec timerSpec;

        const double rate = 1.0 / samplesPerSecond;
        double wholeComponent = 0.0;
        const double fractionalComponent = std::modf(rate, &wholeComponent);

        timerSpec.it_interval.tv_sec = std::llrint(wholeComponent);
        timerSpec.it_interval.tv_nsec = std::llrint(fractionalComponent * 1'000'000'000.0);
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

    Options load_options() {
        return Options::fromString(std::getenv("SWIMPS_OPTIONS"));
    }

    __attribute__((constructor))
    void swimps_preload_constructor() {
        const auto options = load_options();
        swimps::log::setLevelToLog(options.logLevel);
        write_to_log(LogLevel::Debug, "Preload ctor running.");
        write_to_buffer(
            { options.targetProgram.c_str(), options.targetProgram.length() },
            targetProgram
        );

        traceFile = swimps_preload_create_trace_file(traceFilePath, options);
        traceFile.set_proc_maps(get_proc_maps());

        if (swimps_preload_setup_signal_handler() == -1) {
            swimps::log::format_and_write_to_log<1024>(
                swimps::log::LogLevel::Fatal,
                "Could not setup signal handler, errno % (%).",
                errno,
                strerror(errno)
            );

            abort();
        }

        if (swimps::time::create_signal_timer(clockID, SIGPROF, sampleTimer) == -1) {
            swimps::log::format_and_write_to_log<1024>(
                swimps::log::LogLevel::Fatal,
                "Could not create timer, errno % (%).",
                errno,
                strerror(errno)
            );

            abort();
        }

        // Everything from here onwards must be signal safe.

        if (swimps_preload_start_timer(sampleTimer, options.samplesPerSecond) == -1) {
            swimps::log::format_and_write_to_log<1024>(
                swimps::log::LogLevel::Fatal,
                "Could not start timer, errno % (%).",
                errno,
                strerror(errno)
            );

            abort();
        }

        write_to_log(LogLevel::Debug, "Preload ctor finished.");
    }

    __attribute__((destructor))
    void swimps_preload_destructor() {
        write_to_log(LogLevel::Debug, "Preload dtor running.");

        // Disable the signal handler so that non-async signal safe code can be used.
        // TODO: is this safe if the timer fails to get created properly?
        swimps_preload_stop_timer(sampleTimer);

        // Wait until the all in-progress samples are finished.
        while (sigprofRunningFlag.test_and_set());

        auto executable = File::open(
            { targetProgram.data(), strnlen(targetProgram.data(), targetProgram.size()) },
            File::Permissions::ReadOnly
        );

        if (! traceFile.finalise(std::move(executable))) {
            abort();
        }

        write_to_log(LogLevel::Debug, "Preload dtor finished.");
    }
}
