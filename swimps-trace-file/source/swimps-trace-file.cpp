#include "swimps-trace-file.h"
#include "swimps-io.h"
#include "swimps-assert.h"

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cinttypes>
#include <vector>
#include <optional>

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

    int read_trace_file_marker(const int fileDescriptor) {
        char buffer[sizeof swimps_v1_trace_file_marker];
        const ssize_t readReturnCode = read(fileDescriptor, buffer, sizeof buffer);
        if (readReturnCode != sizeof swimps_v1_trace_file_marker) {
            return -1;
        }

        return memcmp(buffer, swimps_v1_trace_file_marker, sizeof swimps_v1_trace_file_marker) == 0 ? 0 : -1;
    }

    EntryKind read_next_entry_kind(const int fileDescriptor) {
        char buffer[swimps_v1_trace_entry_marker_size];
        memset(buffer, 0, sizeof buffer);

        const ssize_t readReturnCode = read(fileDescriptor, buffer, sizeof buffer);

        {
            const char formatBuffer[] = "Entry marker: %s.";

            swimps::log::format_and_write_to_log<64>(
                swimps::log::LogLevel::Debug,
                formatBuffer,
                sizeof formatBuffer,
                buffer
            );
        }

        if (readReturnCode == 0) {
            return EntryKind::EndOfFile;
        }

        if (readReturnCode != swimps_v1_trace_entry_marker_size) {
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

    std::optional<swimps::trace::Sample> read_sample(const int fileDescriptor) {
        swimps::trace::backtrace_id_t backtraceID;

        {
            const auto readReturnCode = read(fileDescriptor, &backtraceID, sizeof backtraceID);
            if (readReturnCode != sizeof backtraceID) {
                return {};
            }
        }

        swimps::time::TimeSpecification timestamp;

        {
            const auto readReturnCode = read(fileDescriptor, &timestamp, sizeof timestamp);
            if (readReturnCode != sizeof timestamp) {
                return {};
            }
        }

        return {{ backtraceID, timestamp }};
    }

    struct RawBacktrace {
        std::vector<void*> addresses;
        swimps::trace::backtrace_id_t id;
    };

    std::optional<RawBacktrace> read_raw_backtrace(const int fileDescriptor) {
        swimps::trace::backtrace_id_t backtraceID;

        {
            const auto readReturnCode = read(fileDescriptor, &backtraceID, sizeof backtraceID);
            if (readReturnCode != sizeof backtraceID) {
                return {};
            }
        }

        swimps::trace::stack_frame_count_t stackFrameCount;

        {
            const auto readReturnCode = read(fileDescriptor, &stackFrameCount, sizeof stackFrameCount);
            if (readReturnCode != sizeof stackFrameCount) {
                return {};
            }
        }

        std::vector<void*> addresses;
        addresses.reserve(stackFrameCount);

        {
            const auto bytesToRead = static_cast<ssize_t>(stackFrameCount * sizeof (void*));
            const auto readReturnCode = read(fileDescriptor, &addresses[0], bytesToRead);
            if (readReturnCode != bytesToRead) {
                return {};
            }
        }

        return {{ addresses, backtraceID }};
    }
}

int swimps::trace::file::create(const char* const path) {

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
    const size_t bytesWritten = swimps::io::write_to_file_descriptor(
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

size_t swimps::trace::file::generate_name(const char* const programName,
                                          const swimps::time::TimeSpecification* const time,
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

size_t swimps::trace::file::add_raw_backtrace(const int targetFileDescriptor,
                                              const swimps::trace::backtrace_id_t backtraceID,
                                              void** entries,
                                              const swimps::trace::stack_frame_count_t entriesCount) {
    size_t bytesWritten = 0;

    bytesWritten += swimps::io::write_to_file_descriptor(swimps_v1_trace_raw_backtrace_marker, sizeof swimps_v1_trace_raw_backtrace_marker, targetFileDescriptor);
    bytesWritten += swimps::io::write_to_file_descriptor(reinterpret_cast<const char*>(&backtraceID), sizeof backtraceID, targetFileDescriptor);
    bytesWritten += swimps::io::write_to_file_descriptor(reinterpret_cast<const char*>(&entriesCount), sizeof entriesCount, targetFileDescriptor);

    for(swimps::trace::stack_frame_count_t i = 0; i < entriesCount; ++i) {
        void* const stackFrame = entries[i];
        bytesWritten += swimps::io::write_to_file_descriptor(reinterpret_cast<const char*>(stackFrame), sizeof stackFrame, targetFileDescriptor);
    }

    return bytesWritten;
}

size_t swimps::trace::file::add_sample(const int targetFileDescriptor, const swimps::trace::Sample* const sample) {
    size_t bytesWritten = 0;

    bytesWritten += swimps::io::write_to_file_descriptor(swimps_v1_trace_sample_marker, sizeof swimps_v1_trace_sample_marker, targetFileDescriptor);
    bytesWritten += swimps::io::write_to_file_descriptor(reinterpret_cast<const char*>(&sample->backtraceID), sizeof sample->backtraceID, targetFileDescriptor);
    bytesWritten += swimps::io::write_to_file_descriptor(reinterpret_cast<const char*>(&sample->timestamp), sizeof sample->timestamp, targetFileDescriptor);

    return bytesWritten;
}

int swimps::trace::file::finalise(const int fileDescriptor) {
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
    if (read_trace_file_marker(fileDescriptor) != 0) {
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
        entryKind = read_next_entry_kind(fileDescriptor);

        {
            const char formatBuffer[] = "Trace file entry kind: %d.";

            swimps::log::format_and_write_to_log<128>(
                swimps::log::LogLevel::Debug,
                formatBuffer,
                sizeof formatBuffer,
                static_cast<int>(entryKind)
            );
        }

        switch(entryKind) {
        case EntryKind::Sample:
            {
                const auto sample = read_sample(fileDescriptor);
                if (!sample) {
                    const char message[] = "Reading sample failed.";

                    swimps::log::write_to_log(
                        swimps::log::LogLevel::Fatal,
                        message,
                        sizeof message
                    );

                    return -1;
                }
            }
            break;
        case EntryKind::RawBacktrace:
            {
                const auto rawBacktrace = read_raw_backtrace(fileDescriptor);
                if (!rawBacktrace) {
                    const char message[] = "Reading raw backtrace failed.";

                    swimps::log::write_to_log(
                        swimps::log::LogLevel::Fatal,
                        message,
                        sizeof message
                    );

                    return -1;
                }
            }
            break;
        case EntryKind::EndOfFile:
            // Nothing needs doing.
            break;
        case EntryKind::Unknown:
            {
                const char message[] = "Unknown entry kind detected, bailing.";

                swimps::log::write_to_log(
                    swimps::log::LogLevel::Debug,
                    message,
                    sizeof message
                );

                return -1;
            }
            break;
        }

        // TODO: read entry, gather symbols and eliminate duplicate backtraces.
    }
    while (entryKind != EntryKind::EndOfFile);

    return 0;
}

