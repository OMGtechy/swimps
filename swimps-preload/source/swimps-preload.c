#include "swimps-time.h"
#include "swimps-log.h"
#include "swimps-trace-file.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Should be set by glibc
extern const char* program_invocation_short_name;

static const clockid_t swimps_preload_clock_id = CLOCK_MONOTONIC_COARSE;

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

__attribute__((constructor))
void swimps_preload_constructor() {
    const int traceFileDescriptor = swimps_preload_create_trace_file();
    (void) traceFileDescriptor;
}
