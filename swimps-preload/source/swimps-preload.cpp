#include "swimps-time/swimps-time.h"
#include "swimps-log/swimps-log.h"
#include "swimps-trace-file/swimps-trace-file.h"
#include "swimps-preload/swimps-preload.h"
#include "swimps-preload/private/sigprof_handler.h"
#include "swimps-option/swimps-option-options.h"

#include <atomic>
#include <cerrno>
#include <cstring>
#include <cmath>

#include <unistd.h>
#include <signal.h>
#include <limits.h>

#include <signalsafe/file.hpp>
#include <signalsafe/memory.hpp>
#include <signalsafe/time.hpp>

#include <signalsampler/sampler.hpp>

using signalsafe::File;
using signalsafe::memory::copy_no_overlap;
using signalsafe::time::now;

using signalsampler::get_backtrace;

using swimps::log::LogLevel;
using swimps::log::write_to_log;
using swimps::preload::get_proc_maps;
using swimps::option::Options;
using swimps::trace::RawSample;
using swimps::trace::TraceFile;

namespace {

    constexpr clockid_t clockID = CLOCK_MONOTONIC;

    std::atomic_flag sigprofRunningFlag = ATOMIC_FLAG_INIT;
    TraceFile traceFile;
    std::array<char, PATH_MAX> targetProgram = { };
    std::array<char, PATH_MAX> traceFilePath = { };
    timer_t sampleTimer;

    template <std::size_t TraceFilePathSize>
    TraceFile swimps_preload_create_trace_file(std::array<char, TraceFilePathSize>& traceFilePath, const Options& options) {
        const size_t pathLength = std::min(options.targetTraceFile.size(), TraceFilePathSize);
        copy_no_overlap(std::span<const char>{ options.targetTraceFile.c_str(), options.targetTraceFile.size() }, traceFilePath);
        return TraceFile::create_and_open({ traceFilePath.data(), pathLength }, TraceFile::Permissions::ReadWrite);
    }

    int swimps_preload_setup_signal_handler() {
        struct sigaction action;
        action.sa_sigaction = swimps::preload::sigprof_handler;
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
        copy_no_overlap(
            std::span<const char>{ options.targetProgram.c_str(), options.targetProgram.length() },
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

        write_to_log(LogLevel::Debug, "Preload dtor finished.");
    }
}

void swimps::preload::sigprof_handler(const int, siginfo_t*, void* context) {
    if (sigprofRunningFlag.test_and_set()) {
        // Drop samples that occur when a sample is already being taken.
        return;
    }

    swimps::trace::RawSample sample;
    sample.timestamp = now(clockID);

    {
        sample.backtrace = get_backtrace<64>(
            static_cast<ucontext_t*>(context)
        );

        traceFile.add_raw_sample(std::move(sample));
    }

    sigprofRunningFlag.clear();
}
