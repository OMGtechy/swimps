#include "swimps-trace-file.h"
#include "swimps-io.h"
#include "swimps-assert.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <inttypes.h>

static const char swimps_v1_trace_file_marker[] = "swimps_v1_trace_file\n";

int swimps_trace_file_create(const char* const path) {

    swimps_assert(path != NULL);

    const int file = open(
        path,
        O_CREAT | O_EXCL | O_RDWR, // Create a file with read and write access.
        S_IRUSR | S_IWUSR // Given read and write permissions to current user.
    );

    if (file == -1) {
        return file;
    }

    // Write out the swimps marker to make such files easily recognisable
    const size_t bytesWritten = swimps_write_to_file_descriptor(
        swimps_v1_trace_file_marker,
        sizeof swimps_v1_trace_file_marker,
        file
    );

    if (bytesWritten != sizeof swimps_v1_trace_file_marker) {
        close(file);
        unlink(path);
        return -1;
    }

    return file;
}

size_t swimps_trace_file_generate_name(const char* const programName,
                                       const swimps_timespec_t* const time,
                                       const pid_t pid,
                                       char* const targetBuffer,
                                       const size_t targetBufferSize) {

    swimps_assert(programName != NULL);
    swimps_assert(time != NULL);
    swimps_assert(targetBuffer != NULL);

    return snprintf(
        targetBuffer,
        targetBufferSize,
        "swimps_trace_%s_%" PRId64 "_%" PRId64 "_%" PRId64,
        programName,
        time->seconds,
        time->nanoseconds,
        (int64_t) pid // There isn't a format specifier for pid_t,
                      // so casting to a large signed type felt like the safest option
    );
}

