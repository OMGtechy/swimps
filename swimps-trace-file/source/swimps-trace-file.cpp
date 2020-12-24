#include "swimps-trace-file.h"

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

#include "swimps-assert.h"
#include "swimps-error.h"
#include "swimps-io.h"
#include "swimps-io-file.h"
#include "swimps-log.h"

using swimps::io::File;
using swimps::io::format_string;

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
                    static_cast<int>(errorCode)
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

    int read_trace_file_marker(File& sourceFile) {
        char buffer[sizeof swimps::trace::file::swimps_v1_trace_file_marker];
        if (! sourceFile.read(buffer)) {
            return -1;
        }

        return memcmp(buffer, swimps::trace::file::swimps_v1_trace_file_marker, sizeof swimps::trace::file::swimps_v1_trace_file_marker) == 0 ? 0 : -1;
    }

    bool goToStartOfFile(File& file) {
        if (! file.seekToStart()) {
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

    bool isSwimpsTraceFile(File& sourceFile) {
        if (read_trace_file_marker(sourceFile) != 0) {
            swimps::log::write_to_log(
                swimps::log::LogLevel::Fatal,
                "Missing swimps trace file marker."
            );

            return false;
        }

        return true;
    }


    EntryKind read_next_entry_kind(File& sourceFile) {
        char buffer[swimps::trace::file::swimps_v1_trace_entry_marker_size];
        memset(buffer, 0, sizeof buffer);

        const auto readReturnCode = sourceFile.read(buffer);

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

    std::optional<swimps::trace::Sample> read_sample(File& sourceFile) {
        swimps::trace::backtrace_id_t backtraceID;

        if (! sourceFile.read(backtraceID)) {
            return {};
        }

        swimps::time::TimeSpecification timestamp;

        if (! sourceFile.read(timestamp.seconds)) {
            return {};
        }

        if (! sourceFile.read(timestamp.nanoseconds)) {
            return {};
        }

        return {{ backtraceID, timestamp }};
    }

    int write_trace_file_marker(File& targetFile) {
        const auto bytesWritten = targetFile.write(swimps::trace::file::swimps_v1_trace_file_marker);

        if (bytesWritten != sizeof swimps::trace::file::swimps_v1_trace_file_marker) {
            targetFile.remove();
            return -1;
        }

        return 0;
    }

    std::variant<swimps::trace::Backtrace, swimps::trace::Sample, swimps::trace::StackFrame, swimps::error::ErrorCode> read_entry(File& sourceFile) {
        using namespace swimps::trace::file; 
        using swimps::error::ErrorCode;

        const auto entryKind = read_next_entry_kind(sourceFile);

        swimps::log::format_and_write_to_log<128>(
            swimps::log::LogLevel::Debug,
            "Trace file entry kind: %.",
            static_cast<int>(entryKind)
        );

        switch(entryKind) {
        case EntryKind::Sample:
            {
                const auto sample = read_sample(sourceFile);
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
                const auto backtrace = read_backtrace(sourceFile);
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
                const auto stackFrame = read_stack_frame(sourceFile);
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

swimps::trace::file::TraceFile::TraceFile(swimps::container::Span<const char> path) noexcept
: File(
    path,
    O_CREAT | O_EXCL | O_RDWR, // Create a file with read and write access.
    S_IRUSR | S_IWUSR // Given read and write permissions to current user.
  ) {
    // Write out the swimps marker to make such files easily recognisable
    const auto writeMarkerReturnValue = write_trace_file_marker(*this);
    swimps_assert(writeMarkerReturnValue != -1);
}

swimps::trace::file::TraceFile::TraceFile(int fileDescriptor, swimps::container::Span<const char> path) noexcept
: File(
    fileDescriptor,
    path
  ) {
    // Write out the swimps marker to make such files easily recognisable
    const auto writeMarkerReturnValue = write_trace_file_marker(*this);
    swimps_assert(writeMarkerReturnValue != -1);
}

size_t swimps::trace::file::generate_name(const char* const programName,
                                          const swimps::time::TimeSpecification& time,
                                          swimps::container::Span<char> target) {

    swimps_assert(programName != NULL);

    return format_string(
        "swimps_trace_%_%_%",
        target,
        programName,
        time.seconds,
        time.nanoseconds
    );
}

std::size_t swimps::trace::file::TraceFile::add_backtrace(const swimps::trace::Backtrace& backtrace) {
    std::size_t bytesWritten = 0;

    bytesWritten += write(swimps::trace::file::swimps_v1_trace_symbolic_backtrace_marker);
    bytesWritten += write(backtrace.id);
    bytesWritten += write(backtrace.stackFrameIDCount);

    for(swimps::trace::stack_frame_count_t i = 0; i < backtrace.stackFrameIDCount; ++i) {
        bytesWritten += write(backtrace.stackFrameIDs[i]);
    }

    return bytesWritten;
}

size_t swimps::trace::file::add_stack_frame(File& targetFile,
                                            const StackFrame& stackFrame) {
    size_t bytesWritten = 0;

    bytesWritten += targetFile.write(swimps::trace::file::swimps_v1_trace_stack_frame_marker);

    const auto  id = stackFrame.id;
    const auto& mangledFunctionName = stackFrame.mangledFunctionName;
    const auto  mangledFunctionNameLength = static_cast<mangled_function_name_length_t>(strnlen(&mangledFunctionName[0], sizeof mangledFunctionName));

    const auto& offset = stackFrame.offset;

    swimps_assert(mangledFunctionNameLength >= 0);

    bytesWritten += targetFile.write(id);
    bytesWritten += targetFile.write(mangledFunctionNameLength);
    bytesWritten += targetFile.write({ &mangledFunctionName[0], static_cast<size_t>(mangledFunctionNameLength) });
    bytesWritten += targetFile.write(offset);

    return bytesWritten;
}

std::optional<swimps::trace::Backtrace> swimps::trace::file::read_backtrace(File& sourceFile) {
    swimps::trace::Backtrace backtrace;

    if (! sourceFile.read(backtrace.id)) {
        return {};
    }

    if (! sourceFile.read(backtrace.stackFrameIDCount)) {
        return {};
    }

    swimps_assert(backtrace.stackFrameIDCount > 0);

    for (swimps::trace::stack_frame_count_t i = 0; i < backtrace.stackFrameIDCount; ++i) {
        if (! sourceFile.read(backtrace.stackFrameIDs[i])) {
            return {};
        }
    }

    return backtrace;
}

std::optional<swimps::trace::StackFrame> swimps::trace::file::read_stack_frame(File& sourceFile) {
    swimps::trace::StackFrame stackFrame;

    if (! sourceFile.read(stackFrame.id)) {
        return {};
    }

    if (! sourceFile.read(stackFrame.mangledFunctionNameLength)) {
        return {};
    }

    {
        const auto bytesToWrite = std::min(
            static_cast<size_t>(stackFrame.mangledFunctionNameLength),
            sizeof swimps::trace::StackFrame::mangledFunctionName
        );

        if (sourceFile.read({
                stackFrame.mangledFunctionName,
                bytesToWrite
            }) != bytesToWrite) {
            return {};
        }
    }

    if (sourceFile.read(stackFrame.offset) != sizeof(stackFrame.offset)) {
        return {};
    }

    return stackFrame;
}

std::size_t swimps::trace::file::TraceFile::add_sample(const swimps::trace::Sample& sample) {
    std::size_t bytesWritten = 0;

    bytesWritten += write(swimps::trace::file::swimps_v1_trace_sample_marker);
    bytesWritten += write(sample.backtraceID);
    bytesWritten += write(sample.timestamp.seconds);
    bytesWritten += write(sample.timestamp.nanoseconds);

    return bytesWritten;
}

int swimps::trace::file::finalise(File& traceFile) {
    if (! (goToStartOfFile(traceFile) && isSwimpsTraceFile(traceFile))) {
        return -1;
    }

    struct BacktraceHash {
        size_t operator() (const swimps::trace::Backtrace& backtrace) const {
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
        size_t operator() (const swimps::trace::StackFrame& stackFrame) const {
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
    size_t preOptimisationSampleCount = 0;
    size_t preOptimisationBacktraceCount = 0;
    size_t preOptimisationStackFrameCount = 0;

    for (auto entry = read_entry(traceFile);
         !stop ;
         entry = read_entry(traceFile)) {
        
        std::visit(
            Visitor{
                stop,
                [&backtraceMap, &preOptimisationBacktraceCount](auto& backtrace){
                    ++preOptimisationBacktraceCount;
                    backtraceMap[backtrace].insert(backtrace.id);
                },
                [&samples, &preOptimisationSampleCount](auto& sample){
                    ++preOptimisationSampleCount;
                    samples.push_back(sample);
                },
                [&stackFrameMap, &preOptimisationStackFrameCount](auto& stackFrame){
                    ++preOptimisationStackFrameCount;
                    stackFrameMap[stackFrame].insert(stackFrame.id);
                }
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
    const auto tempFileDescriptor = mkstemp(tempFileNameBuffer);

    if (tempFileDescriptor == -1) {

        swimps::log::format_and_write_to_log<512>(
            swimps::log::LogLevel::Fatal,
            "Could not create temp file to write finalised trace into, errno % (%).",
            errno,
            strerror(errno)
        );

        return -1;
    }

    TraceFile tempFile{tempFileDescriptor, tempFileNameBuffer};

    swimps::log::format_and_write_to_log<512>(
        swimps::log::LogLevel::Debug,
        "Optimisation: % -> % samples, % -> % backtraces, % -> % stack frames.",
        preOptimisationSampleCount,
        samplesSharingBacktraceID.size(),
        preOptimisationBacktraceCount,
        backtracesSharingStackFrameID.size(),
        preOptimisationStackFrameCount,
        stackFrameMap.size()
    );

    for(const auto& sample : samplesSharingBacktraceID) {
        tempFile.add_sample(sample);
    }

    for(const auto& backtrace : backtracesSharingStackFrameID) {
        tempFile.add_backtrace(backtrace);
    }

    for(const auto& stackFrame: stackFrameMap) {
        add_stack_frame(tempFile, stackFrame.first);
    }

    const swimps::container::Span<const char> traceFilePath = traceFile.getPath();
    const std::string traceFilePathString(&traceFilePath[0], traceFilePath.current_size());
    std::filesystem::copy(tempFileNameBuffer, traceFilePathString, std::filesystem::copy_options::overwrite_existing); 

    return 0;
}

std::optional<swimps::trace::Trace> swimps::trace::file::read(File& sourceFile) {
    if (! (goToStartOfFile(sourceFile) && isSwimpsTraceFile(sourceFile))) {
        return {};
    }

    using swimps::error::ErrorCode;

    swimps::trace::Trace trace;

    bool stop = false;
    for (auto entry = read_entry(sourceFile);
         !stop ;
         entry = read_entry(sourceFile)) {

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

