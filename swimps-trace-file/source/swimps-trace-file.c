#include "swimps-trace-file.h"
#include "swimps-io.h"
#include "swimps-assert.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <inttypes.h>

static const char swimps_v1_trace_file_marker[] = "swimps_v1_trace_file";
static const char swimps_v1_trace_raw_backtrace_marker[] = "\nrb!\n";
static const char swimps_v1_trace_sample_marker[] = "\nsp!\n";

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

size_t swimps_trace_file_add_raw_backtrace(const int targetFileDescriptor,
                                           const swimps_backtrace_id_t backtraceID,
                                           void** entries,
                                           const swimps_stack_frame_count_t entriesCount) {
    size_t bytesWritten = 0;

    bytesWritten += swimps_write_to_file_descriptor(swimps_v1_trace_raw_backtrace_marker, sizeof swimps_v1_trace_raw_backtrace_marker, targetFileDescriptor);
    bytesWritten += swimps_write_to_file_descriptor((char*)&backtraceID, sizeof backtraceID, targetFileDescriptor);
    bytesWritten += swimps_write_to_file_descriptor((char*)&entriesCount, sizeof entriesCount, targetFileDescriptor);

    for(swimps_stack_frame_count_t i = 0; i < entriesCount; ++i) {
        void* const stackFrame = entries[i];
        bytesWritten += swimps_write_to_file_descriptor(stackFrame, sizeof stackFrame, targetFileDescriptor);
    }

    return bytesWritten;
}

size_t swimps_trace_file_add_sample(const int targetFileDescriptor, const swimps_sample_t* const sample) {
    size_t bytesWritten = 0;

    bytesWritten += swimps_write_to_file_descriptor(swimps_v1_trace_sample_marker, sizeof swimps_v1_trace_sample_marker, targetFileDescriptor);
    bytesWritten += swimps_write_to_file_descriptor((char*)&sample->backtraceID, sizeof sample->backtraceID, targetFileDescriptor);
    bytesWritten += swimps_write_to_file_descriptor((char*)&sample->timestamp, sizeof sample->timestamp, targetFileDescriptor);

    return bytesWritten;
}

