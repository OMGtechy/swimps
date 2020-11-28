#include "swimps-trace-file.h"
#include "swimps-io.h"
#include "swimps-assert.h"
#include "swimps-error.h"

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
#include <variant>

#include <unistd.h>
#include <fcntl.h>
#include <execinfo.h>

namespace {
    struct Visitor {
        using BacktraceHandler = std::function<void(swimps::trace::Backtrace&)>;
        using SampleHandler = std::function<void(swimps::trace::Sample&)>;

        Visitor(bool& stopTarget, BacktraceHandler onBacktrace, SampleHandler onSample)
        : m_stopTarget(stopTarget), m_onBacktrace(onBacktrace), m_onSample(onSample) {

        }

        bool& m_stopTarget;
        BacktraceHandler m_onBacktrace;
        SampleHandler m_onSample;

        void operator()(swimps::trace::Sample& sample) const {
            m_onSample(sample);
        }

        void operator()(swimps::trace::Backtrace& backtrace) const {
            m_onBacktrace(backtrace);
        }

        void operator()(swimps::error::ErrorCode errorCode) const {
            m_stopTarget = true;
            switch(errorCode) {
            case swimps::error::ErrorCode::EndOfFile:
                break;
            default:
                swimps::log::format_and_write_to_log<128>(
                    swimps::log::LogLevel::Fatal,
                    "Error reading trace file: %d",
                    errorCode
                );

                break;
            }
        }
    };

    enum class EntryKind : int {
        Unknown,
        EndOfFile,
        Sample,
        SymbolicBacktrace
    };

    int read_trace_file_marker(const int fileDescriptor) {
        char buffer[sizeof swimps::trace::file::swimps_v1_trace_file_marker];
        if (! swimps::io::read_from_file_descriptor(fileDescriptor, buffer)) {
            return -1;
        }

        return memcmp(buffer, swimps::trace::file::swimps_v1_trace_file_marker, sizeof swimps::trace::file::swimps_v1_trace_file_marker) == 0 ? 0 : -1;
    }

    bool goToStartOfFile(const int fileDescriptor) {
        if (lseek(fileDescriptor, 0, SEEK_SET) != 0) {
            swimps::log::format_and_write_to_log<512>(
                swimps::log::LogLevel::Fatal,
                "Could not lseek to start of trace file to begin finalising, errno %d (%s).",
                errno,
                strerror(errno)
            );

            return false;
        }

        return true;
    }

    bool isSwimpsTraceFile(const int fileDescriptor) {
        if (read_trace_file_marker(fileDescriptor) != 0) {
            swimps::log::write_to_log(
                swimps::log::LogLevel::Fatal,
                "Missing swimps trace file marker."
            );

            return false;
        }

        return true;
    }


    EntryKind read_next_entry_kind(const int fileDescriptor) {
        char buffer[swimps::trace::file::swimps_v1_trace_entry_marker_size];
        memset(buffer, 0, sizeof buffer);

        const ssize_t readReturnCode = read(fileDescriptor, buffer, sizeof buffer);

        swimps::log::format_and_write_to_log<64>(
            swimps::log::LogLevel::Debug,
            "Entry marker: %s.",
            buffer
        );

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

        if (! swimps::io::read_from_file_descriptor(fileDescriptor, backtraceID)) {
            return {};
        }

        swimps::time::TimeSpecification timestamp;

        if (! swimps::io::read_from_file_descriptor(fileDescriptor, timestamp.seconds)) {
            return {};
        }

        if (! swimps::io::read_from_file_descriptor(fileDescriptor, timestamp.nanoseconds)) {
            return {};
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

    std::variant<swimps::trace::Backtrace, swimps::trace::Sample, swimps::error::ErrorCode> read_entry(const int fileDescriptor) { 
        using namespace swimps::trace::file; 
        using swimps::error::ErrorCode;

        const auto entryKind = read_next_entry_kind(fileDescriptor);

        swimps::log::format_and_write_to_log<128>(
            swimps::log::LogLevel::Debug,
            "Trace file entry kind: %d.",
            static_cast<int>(entryKind)
        );

        switch(entryKind) {
        case EntryKind::Sample:
            {
                const auto sample = read_sample(fileDescriptor);
                if (!sample) {

                    swimps::log::write_to_log(
                        swimps::log::LogLevel::Fatal,
                        "Reading sample failed."
                    );

                    return ErrorCode::ReadSampleFailed;
                }

                return *sample;
            }
        case EntryKind::SymbolicBacktrace:
            {
                const auto backtrace = read_backtrace(fileDescriptor);
                if (!backtrace) {
                    swimps::log::write_to_log(
                        swimps::log::LogLevel::Fatal,
                        "Reading backtrace failed."
                    );

                    return ErrorCode::ReadBacktraceFailed;
                }

                return *backtrace;
            }
        case EntryKind::EndOfFile:
            return ErrorCode::EndOfFile;
        case EntryKind::Unknown:
        default:
            swimps::log::write_to_log(
                swimps::log::LogLevel::Debug,
                "Unknown entry kind detected, bailing."
            );

            return ErrorCode::UnknownEntryKind;
        }
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
                                          const swimps::time::TimeSpecification& time,
                                          swimps::container::Span<char> target) {

    swimps_assert(programName != NULL);

    return snprintf(
        &target[0],
        target.current_size(),
        "swimps_trace_%s_%" PRId64 "_%" PRId64,
        programName,
        time.seconds,
        time.nanoseconds
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

    if (! swimps::io::read_from_file_descriptor(
            fileDescriptor,
            backtrace.id)) {
        return {};
    }

    if (! swimps::io::read_from_file_descriptor(
            fileDescriptor,
            backtrace.stackFrameCount)) {
        return {};
    }

    swimps_assert(backtrace.stackFrameCount > 0);

    for (swimps::trace::stack_frame_count_t i = 0; i < backtrace.stackFrameCount; ++i) {

        if (! swimps::io::read_from_file_descriptor(
                fileDescriptor,
                backtrace.stackFrames[i].mangledFunctionNameLength)) {
            return {};
        }

        if (! swimps::io::read_from_file_descriptor(
                fileDescriptor,
                {
                    backtrace.stackFrames[i].mangledFunctionName,
                    std::min(
                        static_cast<size_t>(backtrace.stackFrames[i].mangledFunctionNameLength),
                        sizeof swimps::trace::StackFrame::mangledFunctionName
                    )
                })) {
            return {};
        }

        if (! swimps::io::read_from_file_descriptor(
                fileDescriptor,
                backtrace.stackFrames[i].offset)) {
            return {};
        }
    }

    return backtrace;
}

size_t swimps::trace::file::add_sample(const int targetFileDescriptor, const swimps::trace::Sample& sample) {
    size_t bytesWritten = 0;

    bytesWritten += swimps::io::write_to_file_descriptor(swimps::trace::file::swimps_v1_trace_sample_marker, targetFileDescriptor);
    bytesWritten += swimps::io::write_to_file_descriptor(sample.backtraceID, targetFileDescriptor);
    bytesWritten += swimps::io::write_to_file_descriptor(sample.timestamp.seconds, targetFileDescriptor);
    bytesWritten += swimps::io::write_to_file_descriptor(sample.timestamp.nanoseconds, targetFileDescriptor);

    return bytesWritten;
}

int swimps::trace::file::finalise(const int fileDescriptor, const char* const traceFilePath, const size_t traceFilePathSize) {
    if (! (goToStartOfFile(fileDescriptor) && isSwimpsTraceFile(fileDescriptor))) {
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

    using swimps::error::ErrorCode;

    bool stop = false;
    for (auto entry = read_entry(fileDescriptor);
         !stop ;
         entry = read_entry(fileDescriptor)) {
        
        std::visit(
            Visitor{
                stop,
                [&backtraceMap](auto& backtrace){ backtraceMap[backtrace].insert(backtrace.id); },
                [&samples](auto& sample){ samples.push_back(sample); }, 
            },
            entry
        );
    }

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

        swimps::log::format_and_write_to_log<512>(
            swimps::log::LogLevel::Fatal,
            "Could not create temp file to write finalised trace into, errno %d (%s).",
            errno,
            strerror(errno)
        );

        return -1;
    }

    if (write_trace_file_marker(tempFile, tempFileNameBuffer) == -1) {

        swimps::log::format_and_write_to_log<512>(
            swimps::log::LogLevel::Fatal,
            "Could not write trace file marker to finalisation temp file, errno %d (%s).",
            errno,
            strerror(errno)
        );

        return -1;
    }

    for(const auto& sample : samplesSharingBacktraceID) {
        add_sample(tempFile, sample);
    }

    for(const auto& backtrace : backtraceMap) {
        add_backtrace(tempFile, backtrace.first);
    }

    const std::string traceFilePathString(traceFilePath, traceFilePathSize);
    std::filesystem::copy(tempFileNameBuffer, traceFilePathString, std::filesystem::copy_options::overwrite_existing); 

    return 0;
}

std::optional<swimps::trace::Trace> swimps::trace::file::read(int fileDescriptor) {
    if (! (goToStartOfFile(fileDescriptor) && isSwimpsTraceFile(fileDescriptor))) {
        return {};
    }

    using swimps::error::ErrorCode;

    swimps::trace::Trace trace;

    bool stop = false;
    for (auto entry = read_entry(fileDescriptor);
         !stop ;
         entry = read_entry(fileDescriptor)) {
        
        std::visit(
            Visitor{
                stop,
                [&trace](auto& backtrace){ trace.backtraces.push_back(backtrace); },
                [&trace](auto& sample){ trace.samples.push_back(sample); }, 
            },
            entry
        );
    }

    return trace;
}

