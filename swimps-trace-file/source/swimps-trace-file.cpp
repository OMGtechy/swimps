#include "swimps-trace-file.h"
#include "swimps-io.h"
#include "swimps-assert.h"

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cinttypes>

#include <unistd.h>
#include <fcntl.h>

namespace {
    constexpr char swimps_v1_trace_file_marker[] = "swimps_v1_trace_file";

    constexpr size_t swimps_v1_trace_entry_marker_size  = sizeof "\n__!\n";
    constexpr char swimps_v1_trace_raw_backtrace_marker[swimps_v1_trace_entry_marker_size] = "\nrb!\n";
    constexpr char swimps_v1_trace_sample_marker[swimps_v1_trace_entry_marker_size] = "\nsp!\n";

    enum class EntryKind : int {
        Unknown,
        EndOfFile,
        Sample,
        RawBacktrace
    };

    int swimps_trace_file_internal_read_trace_file_marker(const int fileDescriptor) {
        char buffer[sizeof swimps_v1_trace_file_marker];
        const ssize_t readReturnCode = read(fileDescriptor, buffer, sizeof buffer);
        if (readReturnCode != sizeof swimps_v1_trace_file_marker) {
            return -1;
        }

        return memcmp(buffer, swimps_v1_trace_file_marker, sizeof swimps_v1_trace_file_marker) == 0 ? 0 : -1;
    }


    EntryKind swimps_trace_file_internal_read_next_entry_kind(const int fileDescriptor) {
        char buffer[swimps_v1_trace_entry_marker_size];

        const ssize_t readReturnCode = read(fileDescriptor, buffer, sizeof buffer);
        if (readReturnCode != swimps_v1_trace_entry_marker_size) {
            // TODO: sometimes, this will be due to end of file and should return the appropriate value for that.
            return EntryKind::Unknown;
        }

        if (memcmp(buffer, swimps_v1_trace_sample_marker, sizeof swimps_v1_trace_sample_marker) == 0) {
            return EntryKind::Sample;
        }

        if (memcmp(buffer, swimps_v1_trace_raw_backtrace_marker, sizeof swimps_v1_trace_raw_backtrace_marker) == 0) {
            return EntryKind::RawBacktrace;
        }

        return EntryKind::Unknown;
    }
}

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
    bytesWritten += swimps_write_to_file_descriptor(reinterpret_cast<const char*>(&backtraceID), sizeof backtraceID, targetFileDescriptor);
    bytesWritten += swimps_write_to_file_descriptor(reinterpret_cast<const char*>(entriesCount), sizeof entriesCount, targetFileDescriptor);

    for(swimps_stack_frame_count_t i = 0; i < entriesCount; ++i) {
        void* const stackFrame = entries[i];
        bytesWritten += swimps_write_to_file_descriptor(reinterpret_cast<const char*>(stackFrame), sizeof stackFrame, targetFileDescriptor);
    }

    return bytesWritten;
}

size_t swimps_trace_file_add_sample(const int targetFileDescriptor, const swimps_sample_t* const sample) {
    size_t bytesWritten = 0;

    bytesWritten += swimps_write_to_file_descriptor(swimps_v1_trace_sample_marker, sizeof swimps_v1_trace_sample_marker, targetFileDescriptor);
    bytesWritten += swimps_write_to_file_descriptor(reinterpret_cast<const char*>(&sample->backtraceID), sizeof sample->backtraceID, targetFileDescriptor);
    bytesWritten += swimps_write_to_file_descriptor(reinterpret_cast<const char*>(&sample->timestamp), sizeof sample->timestamp, targetFileDescriptor);

    return bytesWritten;
}

int swimps_trace_file_finalise(const int fileDescriptor) {
    // Go to the start of the file.
    if (lseek(fileDescriptor, 0, SEEK_SET) != 0) {
        const char formatBuffer[] = "Could not lseek to start of trace file to begin finalising, errno %d (%s).";

        swimps::log::format_and_write_to_log<512>(
            swimps::log::LogLevel::Fatal,
            formatBuffer,
            sizeof formatBuffer,
            errno,
            strerror(errno)
        );

        return -1;
    }

    // Make sure this is actually a swimps trace file
    if (swimps_trace_file_internal_read_trace_file_marker(fileDescriptor) != 0) {
        const char message[] = "Missing swimps trace file marker.";

        swimps::log::write_to_log(
            swimps::log::LogLevel::Fatal,
            message,
            sizeof message
        );

        return -1;
    }

    auto entryKind = EntryKind::Unknown;

    do {
        entryKind = swimps_trace_file_internal_read_next_entry_kind(fileDescriptor);

        {
            const char formatBuffer[] = "Trace file entry kind: %d.";

            swimps::log::format_and_write_to_log<128>(
                swimps::log::LogLevel::Debug,
                formatBuffer,
                sizeof formatBuffer,
                static_cast<int>(entryKind)
            );
        }

        if (entryKind == EntryKind::Unknown) {
            const char message[] = "Unknown entry kind detected, bailing.";

            swimps::log::write_to_log(
                swimps::log::LogLevel::Debug,
                message,
                sizeof message
            );

            return -1;
        }

        // TODO: read entry, gather symbols and eliminate duplicate backtraces.
    }
    while (entryKind != EntryKind::EndOfFile);

    return 0;
}

