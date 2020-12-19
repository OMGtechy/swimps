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
        using StackFrameHandler = std::function<void(swimps::trace::StackFrame&)>;

        Visitor(bool& stopTarget, BacktraceHandler onBacktrace, SampleHandler onSample, StackFrameHandler onStackFrame)
        : m_stopTarget(stopTarget), m_onBacktrace(onBacktrace), m_onSample(onSample), m_onStackFrame(onStackFrame) {

        }

        bool& m_stopTarget;
        BacktraceHandler m_onBacktrace;
        SampleHandler m_onSample;
        StackFrameHandler m_onStackFrame;

        void operator()(swimps::trace::Sample& sample) const {
            m_onSample(sample);
        }

        void operator()(swimps::trace::Backtrace& backtrace) const {
            m_onBacktrace(backtrace);
        }

        void operator()(swimps::trace::StackFrame& stackFrame) const {
            m_onStackFrame(stackFrame);
        }

        void operator()(swimps::error::ErrorCode errorCode) const {
            m_stopTarget = true;
            switch(errorCode) {
            case swimps::error::ErrorCode::EndOfFile:
                break;
            default:
                swimps::log::format_and_write_to_log<128>(
                    swimps::log::LogLevel::Fatal,
                    "Error reading trace file: %",
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
        SymbolicBacktrace,
        StackFrame
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
                "Could not lseek to start of trace file to begin finalising, errno % (%).",
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
            "Entry marker: %.",
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

        if (memcmp(buffer, swimps::trace::file::swimps_v1_trace_stack_frame_marker, sizeof swimps::trace::file::swimps_v1_trace_stack_frame_marker) == 0) {
            return EntryKind::StackFrame;
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

    std::variant<swimps::trace::Backtrace, swimps::trace::Sample, swimps::trace::StackFrame, swimps::error::ErrorCode> read_entry(const int fileDescriptor) { 
        using namespace swimps::trace::file; 
        using swimps::error::ErrorCode;

        const auto entryKind = read_next_entry_kind(fileDescriptor);

        swimps::log::format_and_write_to_log<128>(
            swimps::log::LogLevel::Debug,
            "Trace file entry kind: %.",
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
        case EntryKind::StackFrame:
            {
                const auto stackFrame = read_stack_frame(fileDescriptor);
                if (!stackFrame) {
                    swimps::log::write_to_log(
                        swimps::log::LogLevel::Fatal,
                        "Reading stack frame failed."
                    );

                    return ErrorCode::ReadStackFrameFailed;
                }

                return *stackFrame;
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
    bytesWritten += swimps::io::write_to_file_descriptor(backtrace.stackFrameIDCount, targetFileDescriptor);

    for(swimps::trace::stack_frame_count_t i = 0; i < backtrace.stackFrameIDCount; ++i) {
        bytesWritten += swimps::io::write_to_file_descriptor(backtrace.stackFrameIDs[i], targetFileDescriptor);
    }

    return bytesWritten;
}

size_t swimps::trace::file::add_stack_frame(const int targetFileDescriptor,
                                            const StackFrame& stackFrame) {
    size_t bytesWritten = 0;

    bytesWritten += swimps::io::write_to_file_descriptor(swimps::trace::file::swimps_v1_trace_stack_frame_marker, targetFileDescriptor);

    const auto  id = stackFrame.id;
    const auto& mangledFunctionName = stackFrame.mangledFunctionName;
    const auto  mangledFunctionNameLength = static_cast<mangled_function_name_length_t>(strnlen(&mangledFunctionName[0], sizeof mangledFunctionName));

    const auto& offset = stackFrame.offset;

    swimps_assert(mangledFunctionNameLength >= 0);

    bytesWritten += swimps::io::write_to_file_descriptor(
        id,
        targetFileDescriptor
    );

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
            backtrace.stackFrameIDCount)) {
        return {};
    }

    swimps_assert(backtrace.stackFrameIDCount > 0);

    for (swimps::trace::stack_frame_count_t i = 0; i < backtrace.stackFrameIDCount; ++i) {
        if (! swimps::io::read_from_file_descriptor(
                fileDescriptor,
                backtrace.stackFrameIDs[i])) {
            return {};
        }
    }

    return backtrace;
}

std::optional<swimps::trace::StackFrame> swimps::trace::file::read_stack_frame(const int fileDescriptor) {
    swimps::trace::StackFrame stackFrame;

    if (! swimps::io::read_from_file_descriptor(
            fileDescriptor,
            stackFrame.id)) {
        return {};
    }

    if (! swimps::io::read_from_file_descriptor(
            fileDescriptor,
            stackFrame.mangledFunctionNameLength)) {
        return {};
    }

    if (! swimps::io::read_from_file_descriptor(
            fileDescriptor,
            {
                stackFrame.mangledFunctionName,
                std::min(
                    static_cast<size_t>(stackFrame.mangledFunctionNameLength),
                    sizeof swimps::trace::StackFrame::mangledFunctionName
                )
            })) {
        return {};
    }

    if (! swimps::io::read_from_file_descriptor(
            fileDescriptor,
            stackFrame.offset)) {
        return {};
    }

    return stackFrame;
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
            swimps_assert(backtrace.stackFrameIDCount > 0);
            return std::hash<swimps::trace::stack_frame_id_t>{}(backtrace.stackFrameIDs[0]);
        }
    };

    struct BacktraceEqual {
        bool operator() (
            const swimps::trace::Backtrace& lhs,
            const swimps::trace::Backtrace& rhs
        ) const {
            if (lhs.stackFrameIDCount != rhs.stackFrameIDCount) {
                return false;
            }

            for (decltype(lhs.stackFrameIDCount) i = 0; i < lhs.stackFrameIDCount; ++i) {
                if (lhs.stackFrameIDs[i] != rhs.stackFrameIDs[i]) {
                    return false;
                }
            }

            return true;
        }
    };

    struct StackFrameHash {
        bool operator() (const swimps::trace::StackFrame& stackFrame) const {
            return std::hash<const char*>{}(stackFrame.mangledFunctionNameLength == 0 ? nullptr : &stackFrame.mangledFunctionName[0]);
        }
    };

    struct StackFrameEqual {
        bool operator() (
            const swimps::trace::StackFrame& lhs,
            const swimps::trace::StackFrame& rhs
        ) const {
            return lhs.isEquivalentTo(rhs);
        }
    };

    std::vector<swimps::trace::Sample> samples;

    std::unordered_map<
        Backtrace,
        std::unordered_set<swimps::trace::backtrace_id_t>,
        BacktraceHash,
        BacktraceEqual> backtraceMap;

    std::unordered_map<
        StackFrame,
        std::unordered_set<swimps::trace::stack_frame_id_t>,
        StackFrameHash,
        StackFrameEqual> stackFrameMap;

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
                [&stackFrameMap](auto& stackFrame){ stackFrameMap[stackFrame].insert(stackFrame.id); }
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

    std::vector<swimps::trace::Backtrace> backtracesSharingStackFrameID;
    for(const auto& [oldBacktrace, unused] : backtraceMap) {

        swimps::trace::Backtrace newBacktrace;
        newBacktrace.id = oldBacktrace.id;

        for(stack_frame_count_t i = 0; i < oldBacktrace.stackFrameIDCount; ++i) {
            const auto& stackFrameID = oldBacktrace.stackFrameIDs[i];
            const auto matchingStackFrameIter = std::find_if(
                stackFrameMap.cbegin(),
                stackFrameMap.cend(),
                [stackFrameID](const auto& stackFramePair) {
                    return stackFramePair.second.contains(stackFrameID);
                }
            );

            swimps_assert(matchingStackFrameIter != stackFrameMap.cend());

            newBacktrace.stackFrameIDs[i] = matchingStackFrameIter->first.id;
            newBacktrace.stackFrameIDCount += 1;
        }

        swimps_assert(newBacktrace.stackFrameIDCount == oldBacktrace.stackFrameIDCount);

        backtracesSharingStackFrameID.push_back(newBacktrace);
    }

    char tempFileNameBuffer[] = "/tmp/swimps_finalise_temp_file_XXXXXX";
    const auto tempFile = mkstemp(tempFileNameBuffer);

    if (tempFile == -1) {

        swimps::log::format_and_write_to_log<512>(
            swimps::log::LogLevel::Fatal,
            "Could not create temp file to write finalised trace into, errno % (%).",
            errno,
            strerror(errno)
        );

        return -1;
    }

    if (write_trace_file_marker(tempFile, tempFileNameBuffer) == -1) {

        swimps::log::format_and_write_to_log<512>(
            swimps::log::LogLevel::Fatal,
            "Could not write trace file marker to finalisation temp file, errno % (%).",
            errno,
            strerror(errno)
        );

        return -1;
    }

    for(const auto& sample : samplesSharingBacktraceID) {
        add_sample(tempFile, sample);
    }

    for(const auto& backtrace : backtracesSharingStackFrameID) {
        add_backtrace(tempFile, backtrace);
    }

    for(const auto& stackFrame: stackFrameMap) {
        add_stack_frame(tempFile, stackFrame.first);
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
                [&trace](auto& stackFrame){ trace.stackFrames.push_back(stackFrame); }
            },
            entry
        );
    }

    return trace;
}

