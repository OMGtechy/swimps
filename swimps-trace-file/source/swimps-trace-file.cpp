#include "swimps-trace-file.h"
#include "swimps-io.h"
#include "swimps-assert.h"

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cinttypes>
#include <vector>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <string>
#include <filesystem>

#include <unistd.h>
#include <fcntl.h>
#include <execinfo.h>

namespace {
    constexpr char swimps_v1_trace_file_marker[] = "swimps_v1_trace_file";

    constexpr size_t swimps_v1_trace_entry_marker_size  = sizeof "\n__!\n";
    constexpr char swimps_v1_trace_raw_backtrace_marker[swimps_v1_trace_entry_marker_size] = "\nrb!\n";
    constexpr char swimps_v1_trace_symbolic_backtrace_marker[swimps_v1_trace_entry_marker_size] = "\nsb!\n";
    constexpr char swimps_v1_trace_sample_marker[swimps_v1_trace_entry_marker_size] = "\nsp!\n";

    enum class EntryKind : int {
        Unknown,
        EndOfFile,
        Sample,
        RawBacktrace,
        SymbolicBacktrace
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

        if (memcmp(buffer, swimps_v1_trace_symbolic_backtrace_marker, sizeof swimps_v1_trace_symbolic_backtrace_marker) == 0) {
            return EntryKind::SymbolicBacktrace;
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
        addresses.resize(stackFrameCount);

        {
            const auto bytesToRead = static_cast<ssize_t>(stackFrameCount * sizeof (void*));
            const auto readReturnCode = read(fileDescriptor, &addresses[0], bytesToRead);
            if (readReturnCode != bytesToRead) {
                return {};
            }
        }

        swimps_assert(addresses.size() > 0);

        return {{ addresses, backtraceID }};
    }

    int write_trace_file_marker(const int fileDescriptor, const char* const path) {
        const size_t bytesWritten = swimps::io::write_to_file_descriptor(
            swimps_v1_trace_file_marker,
            sizeof swimps_v1_trace_file_marker,
            fileDescriptor
        );

        if (bytesWritten != sizeof swimps_v1_trace_file_marker) {
            close(fileDescriptor);
            unlink(path);
            return -1;
        }

        return 0;
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
    if (write_trace_file_marker(file, path) == -1) {
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

int swimps::trace::file::finalise(const int fileDescriptor, const char* const traceFilePath, const size_t traceFilePathSize) {
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

    struct RawBacktraceHash {
        bool operator() (const RawBacktrace& rawBacktrace) const {
            swimps_assert(rawBacktrace.addresses.size() > 0);
            return std::hash<void*>{}((rawBacktrace.addresses[0]));
        }
    };

    struct RawBacktraceEqual {
        bool operator() (
            const RawBacktrace& lhs,
            const RawBacktrace& rhs
        ) const {
            return lhs.addresses == rhs.addresses;
        }
    };

    std::vector<swimps::trace::Sample> samples;

    std::unordered_map<
        RawBacktrace,
        std::unordered_set<swimps::trace::backtrace_id_t>,
        RawBacktraceHash,
        RawBacktraceEqual> rawBacktraceMap;

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

                samples.push_back(*sample);
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

                rawBacktraceMap[*rawBacktrace].insert(rawBacktrace->id);
            }
            break;
        case EntryKind::EndOfFile:
            // Nothing needs doing.
            break;
        case EntryKind::SymbolicBacktrace:
            {
                const char message[] = "Unexpected symbolic backtrace, bailing.";

                swimps::log::write_to_log(
                    swimps::log::LogLevel::Debug,
                    message,
                    sizeof message
                );

                return -1;
            }
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

    }
    while (entryKind != EntryKind::EndOfFile);

    std::vector<swimps::trace::Sample> samplesSharingBacktraceID;
    for(const auto& sample : samples) {
        const auto matchingRawBacktraceIter = std::find_if(
            rawBacktraceMap.cbegin(),
            rawBacktraceMap.cend(),
            [&sample](const auto& rawBacktracePair){
                return rawBacktracePair.second.contains(sample.backtraceID);
            }
        );

        swimps_assert(matchingRawBacktraceIter != rawBacktraceMap.cend());

        samplesSharingBacktraceID.emplace_back(
            matchingRawBacktraceIter->first.id,
            sample.timestamp
        );
    }

    char tempFileNameBuffer[] = "/tmp/swimps_finalise_temp_file_XXXXXX";
    const auto tempFile = mkstemp(tempFileNameBuffer);

    if (tempFile == -1) {
        const char formatBuffer[] = "Could not create temp file to write finalised trace into, errno %d (%s).";

        swimps::log::format_and_write_to_log<512>(
            swimps::log::LogLevel::Fatal,
            formatBuffer,
            sizeof formatBuffer,
            errno,
            strerror(errno)
        );

        return -1;
    }

    if (write_trace_file_marker(tempFile, tempFileNameBuffer) == -1) {
        const char formatBuffer[] = "Could not write trace file marker to finalisation temp file, errno %d (%s).";

        swimps::log::format_and_write_to_log<512>(
            swimps::log::LogLevel::Fatal,
            formatBuffer,
            sizeof formatBuffer,
            errno,
            strerror(errno)
        );

        return -1;
    }

    for(const auto& sample : samplesSharingBacktraceID) {
        add_sample(tempFile, &sample);
    }

    for(const auto& rawBacktrace : rawBacktraceMap) {
        {
            const auto bytesWritten = swimps::io::write_to_file_descriptor(
                swimps_v1_trace_symbolic_backtrace_marker,
                sizeof swimps_v1_trace_symbolic_backtrace_marker,
                tempFile
            );

            if (bytesWritten != sizeof swimps_v1_trace_symbolic_backtrace_marker) {
                const char formatBuffer[] = "Could not symbolic backtrace marker, errno %d (%s).";

                swimps::log::format_and_write_to_log<512>(
                    swimps::log::LogLevel::Fatal,
                    formatBuffer,
                    sizeof formatBuffer,
                    errno,
                    strerror(errno)
                );

                return -1;
            }
        }

        backtrace_symbols_fd(rawBacktrace.first.addresses.data(), rawBacktrace.first.addresses.size(), tempFile);
    }

    const std::string traceFilePathString(traceFilePath, traceFilePathSize);
    std::filesystem::copy(tempFileNameBuffer, traceFilePathString, std::filesystem::copy_options::overwrite_existing); 

    return 0;
}

