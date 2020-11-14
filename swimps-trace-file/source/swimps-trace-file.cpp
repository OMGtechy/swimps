#include "swimps-trace-file.h"
#include "swimps-io.h"
#include "swimps-assert.h"

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cinttypes>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <string>
#include <filesystem>

#include <unistd.h>
#include <fcntl.h>
#include <execinfo.h>

namespace {
    enum class EntryKind : int {
        Unknown,
        EndOfFile,
        Sample,
        SymbolicBacktrace
    };

    int read_trace_file_marker(const int fileDescriptor) {
        char buffer[sizeof swimps::trace::file::swimps_v1_trace_file_marker];
        const ssize_t readReturnCode = read(fileDescriptor, buffer, sizeof buffer);
        if (readReturnCode != sizeof swimps::trace::file::swimps_v1_trace_file_marker) {
            return -1;
        }

        return memcmp(buffer, swimps::trace::file::swimps_v1_trace_file_marker, sizeof swimps::trace::file::swimps_v1_trace_file_marker) == 0 ? 0 : -1;
    }

    EntryKind read_next_entry_kind(const int fileDescriptor) {
        char buffer[swimps::trace::file::swimps_v1_trace_entry_marker_size];
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

        if (readReturnCode != swimps::trace::file::swimps_v1_trace_entry_marker_size) {
            return EntryKind::Unknown;
        }

        if (memcmp(buffer, swimps::trace::file::swimps_v1_trace_sample_marker, sizeof swimps::trace::file::swimps_v1_trace_sample_marker) == 0) {
            return EntryKind::Sample;
        }

        if (memcmp(buffer, swimps::trace::file::swimps_v1_trace_symbolic_backtrace_marker, sizeof swimps::trace::file::swimps_v1_trace_symbolic_backtrace_marker) == 0) {
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
            const auto readReturnCode = read(fileDescriptor, &timestamp.seconds, sizeof(timestamp.seconds));
            if (readReturnCode != sizeof(timestamp.seconds)) {
                return {};
            }
        }

        {
            const auto readReturnCode = read(fileDescriptor, &timestamp.nanoseconds, sizeof(timestamp.nanoseconds));
            if (readReturnCode != sizeof(timestamp.nanoseconds)) {
                return {};
            }
        }

        return {{ backtraceID, timestamp }};
    }

    int write_trace_file_marker(const int fileDescriptor, const char* const path) {
        const size_t bytesWritten = swimps::io::write_to_file_descriptor(
            swimps::trace::file::swimps_v1_trace_file_marker,
            fileDescriptor
        );

        if (bytesWritten != sizeof swimps::trace::file::swimps_v1_trace_file_marker) {
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

size_t swimps::trace::file::add_backtrace(const int targetFileDescriptor,
                                          const swimps::trace::Backtrace& backtrace) {
    size_t bytesWritten = 0;

    bytesWritten += swimps::io::write_to_file_descriptor(swimps::trace::file::swimps_v1_trace_symbolic_backtrace_marker, targetFileDescriptor);
    bytesWritten += swimps::io::write_to_file_descriptor(backtrace.id, targetFileDescriptor);
    bytesWritten += swimps::io::write_to_file_descriptor(backtrace.stackFrameCount, targetFileDescriptor);

    for(swimps::trace::stack_frame_count_t i = 0; i < backtrace.stackFrameCount; ++i) {
        const auto& stackFrame = backtrace.stackFrames[i];
        const auto& mangledFunctionName = stackFrame.mangledFunctionName;
        const auto  mangledFunctionNameLength = static_cast<mangled_function_name_length_t>(strnlen(&mangledFunctionName[0], sizeof mangledFunctionName));

        const auto& offset = stackFrame.offset;

        swimps_assert(mangledFunctionNameLength >= 0);

        bytesWritten += swimps::io::write_to_file_descriptor(
            mangledFunctionNameLength,
            targetFileDescriptor
        );

        bytesWritten += swimps::io::write_to_file_descriptor(
            { &mangledFunctionName[0], static_cast<size_t>(mangledFunctionNameLength) },
            targetFileDescriptor
        );

        bytesWritten += swimps::io::write_to_file_descriptor(
            offset,
            targetFileDescriptor
        );
    }

    return bytesWritten;
}

std::optional<swimps::trace::Backtrace> swimps::trace::file::read_backtrace(const int fileDescriptor) {
    swimps::trace::Backtrace backtrace;

    {
        const auto readReturnCode = read(fileDescriptor, &backtrace.id, sizeof backtrace.id);
        if (readReturnCode != sizeof backtrace.id) {
            return {};
        }
    }

    {
        const auto readReturnCode = read(
            fileDescriptor,
            &backtrace.stackFrameCount,
            sizeof backtrace.stackFrameCount
        );

        if (readReturnCode != sizeof backtrace.stackFrameCount) {
            return {};
        }
    }

    swimps_assert(backtrace.stackFrameCount > 0);

    for (swimps::trace::stack_frame_count_t i = 0; i < backtrace.stackFrameCount; ++i) {

        {
            const auto bytesToRead = static_cast<ssize_t>(sizeof backtrace.stackFrames[i].mangledFunctionNameLength);
            const auto readReturnCode = read(
                fileDescriptor,
                &backtrace.stackFrames[i].mangledFunctionNameLength,
                bytesToRead
            );

            if (readReturnCode != bytesToRead) {
                return {};
            }
        }

        {
            const auto bytesToRead = std::min(
                static_cast<ssize_t>(backtrace.stackFrames[i].mangledFunctionNameLength),
                static_cast<ssize_t>(sizeof swimps::trace::StackFrame::mangledFunctionName)
            );

            const auto readReturnCode = read(
                fileDescriptor,
                &backtrace.stackFrames[i].mangledFunctionName[0],
                bytesToRead
            );

            if (readReturnCode != bytesToRead) {
                return {};
            }
        }

        {
            const auto bytesToRead = sizeof swimps::trace::StackFrame::offset;
            const auto readReturnCode = read(
                fileDescriptor,
                &backtrace.stackFrames[i].offset,
                bytesToRead
            );

            if (readReturnCode != bytesToRead) {
                return {};
            }
        }
    }


    return backtrace;
}

size_t swimps::trace::file::add_sample(const int targetFileDescriptor, const swimps::trace::Sample* const sample) {
    size_t bytesWritten = 0;

    bytesWritten += swimps::io::write_to_file_descriptor(swimps::trace::file::swimps_v1_trace_sample_marker, targetFileDescriptor);
    bytesWritten += swimps::io::write_to_file_descriptor(sample->backtraceID, targetFileDescriptor);
    bytesWritten += swimps::io::write_to_file_descriptor(sample->timestamp.seconds, targetFileDescriptor);
    bytesWritten += swimps::io::write_to_file_descriptor(sample->timestamp.nanoseconds, targetFileDescriptor);

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

    struct BacktraceHash {
        bool operator() (const swimps::trace::Backtrace& backtrace) const {
            swimps_assert(backtrace.stackFrameCount > 0);
            return std::hash<const char*>{}((&backtrace.stackFrames[0].mangledFunctionName[0]));
        }
    };

    struct BacktraceEqual {
        bool operator() (
            const swimps::trace::Backtrace& lhs,
            const swimps::trace::Backtrace& rhs
        ) const {
            if (lhs.stackFrameCount != rhs.stackFrameCount) {
                return false;
            }

            for (decltype(lhs.stackFrameCount) i = 0; i < lhs.stackFrameCount; ++i) {
                if (lhs.stackFrames[i] != rhs.stackFrames[i]) {
                    return false;
                }
            }

            return true;
        }
    };

    std::vector<swimps::trace::Sample> samples;

    std::unordered_map<
        Backtrace,
        std::unordered_set<swimps::trace::backtrace_id_t>,
        BacktraceHash,
        BacktraceEqual> backtraceMap;

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
        case EntryKind::SymbolicBacktrace:
            {
                const auto backtrace = read_backtrace(fileDescriptor);
                if (!backtrace) {
                    const char message[] = "Reading backtrace failed.";

                    swimps::log::write_to_log(
                        swimps::log::LogLevel::Fatal,
                        message,
                        sizeof message
                    );

                    return -1;
                }

                backtraceMap[*backtrace].insert(backtrace->id);
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

    }
    while (entryKind != EntryKind::EndOfFile);

    std::vector<swimps::trace::Sample> samplesSharingBacktraceID;
    for(const auto& sample : samples) {
        const auto matchingBacktraceIter = std::find_if(
            backtraceMap.cbegin(),
            backtraceMap.cend(),
            [&sample](const auto& backtracePair){
                return backtracePair.second.contains(sample.backtraceID);
            }
        );

        swimps_assert(matchingBacktraceIter != backtraceMap.cend());

        samplesSharingBacktraceID.push_back({
            matchingBacktraceIter->first.id,
            sample.timestamp
        });
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

    for(const auto& backtrace : backtraceMap) {
        add_backtrace(tempFile, backtrace.first);
    }

    const std::string traceFilePathString(traceFilePath, traceFilePathSize);
    std::filesystem::copy(tempFileNameBuffer, traceFilePathString, std::filesystem::copy_options::overwrite_existing); 

    return 0;
}

