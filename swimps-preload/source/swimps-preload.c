#include "swimps-time.h"
#include "swimps-log.h"
#include "swimps-trace-file.h"
#include "swimps-io.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

// Should be set by glibc
extern const char* program_invocation_short_name;

static const clockid_t swimps_preload_clock_id = CLOCK_MONOTONIC;
static atomic_flag swimps_preload_sigprof_running_flag = ATOMIC_FLAG_INIT;
static int swimps_preload_trace_file = -1;

static void swimps_preload_sigprof_handler(int signalNumber) {
    (void) signalNumber;

    if (atomic_flag_test_and_set(&swimps_preload_sigprof_running_flag)) {
        // Drop samples that occur when a sample is already being taken.
        return;
    }

    const char message[] = "Signal being handled.";
    swimps_write_to_log(
        SWIMPS_LOG_LEVEL_DEBUG,
        message,
        sizeof message
    );

    atomic_flag_clear(&swimps_preload_sigprof_running_flag);
}

static int swimps_preload_create_trace_file() {
    swimps_timespec_t time;
    if (swimps_gettime(swimps_preload_clock_id, &time) == -1) {

        const char message[] = "Could not get time to generate trace file name.";
        swimps_write_to_log(
            SWIMPS_LOG_LEVEL_FATAL,
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
        swimps_write_to_log(
            SWIMPS_LOG_LEVEL_FATAL,
            message,
            sizeof message
        );

        abort();
    }

    const int file = swimps_trace_file_create(traceFileNameBuffer);
    if (file == -1) {

        const char message[] = "Could not create trace file.";
        swimps_write_to_log(
            SWIMPS_LOG_LEVEL_FATAL,
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

__attribute__((constructor))
void swimps_preload_constructor() {
    swimps_preload_trace_file = swimps_preload_create_trace_file();

    if (swimps_preload_setup_signal_handler() == -1) {
        const char formatBuffer[] = "Could not setup signal handler, errno %d (%s).";
        char targetBuffer[1024] = { 0 };

        swimps_format_and_write_to_log(
            SWIMPS_LOG_LEVEL_FATAL,
            formatBuffer,
            sizeof formatBuffer,
            targetBuffer,
            sizeof targetBuffer,
            errno,
            strerror(errno)
        );

        abort();
    }

    timer_t timer;
    if (swimps_create_signal_timer(swimps_preload_clock_id, SIGPROF, &timer) == -1) {
        const char formatBuffer[] = "Could not create timer, errno %d (%s).";
        char targetBuffer[1024] = { 0 };

        swimps_format_and_write_to_log(
            SWIMPS_LOG_LEVEL_FATAL,
            formatBuffer,
            sizeof formatBuffer,
            targetBuffer,
            sizeof targetBuffer,
            errno,
            strerror(errno)
        );

        abort();
    }

    if (swimps_preload_start_timer(timer) == -1) {
        const char formatBuffer[] = "Could not start timer, errno %d (%s).";
        char targetBuffer[1024] = { 0 };

        swimps_format_and_write_to_log(
            SWIMPS_LOG_LEVEL_FATAL,
            formatBuffer,
            sizeof formatBuffer,
            targetBuffer,
            sizeof targetBuffer,
            errno,
            strerror(errno)
        );

        abort();
    }
}
