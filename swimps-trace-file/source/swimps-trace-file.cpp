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

#include <fcntl.h>

#include "swimps-assert.h"
#include "swimps-io.h"
#include "swimps-log.h"

using swimps::error::ErrorCode;
using swimps::container::Span;
using swimps::io::File;
using swimps::io::format_string;
using swimps::log::format_and_write_to_log;
using swimps::log::LogLevel;
using swimps::log::write_to_log;
using swimps::time::TimeSpecification;
using swimps::trace::Backtrace;
using swimps::trace::backtrace_id_t;
using swimps::trace::mangled_function_name_length_t;
using swimps::trace::Sample;
using swimps::trace::StackFrame;
using swimps::trace::stack_frame_count_t;
using swimps::trace::stack_frame_id_t;
using swimps::trace::Trace;

namespace {
    using namespace swimps::trace::file;

    constexpr size_t swimps_v1_trace_entry_marker_size = 6;
    constexpr char swimps_v1_trace_file_marker[swimps_v1_trace_entry_marker_size] = "s_v1\n";
    constexpr char swimps_v1_trace_symbolic_backtrace_marker[swimps_v1_trace_entry_marker_size] = "\nsb!\n";
    constexpr char swimps_v1_trace_sample_marker[swimps_v1_trace_entry_marker_size] = "\nsp!\n";
    constexpr char swimps_v1_trace_stack_frame_marker[swimps_v1_trace_entry_marker_size] = "\nsf!\n";

    struct Visitor {
        using BacktraceHandler = std::function<void(Backtrace&)>;
        using SampleHandler = std::function<void(Sample&)>;
        using StackFrameHandler = std::function<void(StackFrame&)>;

        Visitor(bool& stopTarget, BacktraceHandler onBacktrace, SampleHandler onSample, StackFrameHandler onStackFrame)
        : m_stopTarget(stopTarget), m_onBacktrace(onBacktrace), m_onSample(onSample), m_onStackFrame(onStackFrame) {

        }

        bool& m_stopTarget;
        BacktraceHandler m_onBacktrace;
        SampleHandler m_onSample;
        StackFrameHandler m_onStackFrame;

        void operator()(Sample& sample) const {
            m_onSample(sample);
        }

        void operator()(Backtrace& backtrace) const {
            m_onBacktrace(backtrace);
        }

        void operator()(StackFrame& stackFrame) const {
            m_onStackFrame(stackFrame);
        }

        void operator()(ErrorCode errorCode) const {
            m_stopTarget = true;
            switch(errorCode) {
            case ErrorCode::EndOfFile:
                break;
            default:
                format_and_write_to_log<128>(
                    LogLevel::Fatal,
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

    int read_trace_file_marker(TraceFile& traceFile) {
        char buffer[sizeof swimps_v1_trace_file_marker];
        if (! traceFile.read(buffer)) {
            return -1;
        }

        return memcmp(buffer, swimps_v1_trace_file_marker, sizeof swimps_v1_trace_file_marker) == 0 ? 0 : -1;
    }

    bool goToStartOfFile(TraceFile& file) {
        if (! file.seekToStart()) {
            format_and_write_to_log<512>(
                LogLevel::Fatal,
                "Could not lseek to start of trace file to begin finalising, errno % (%).",
                errno,
                strerror(errno)
            );

            return false;
        }

        return true;
    }

    bool isSwimpsTraceFile(TraceFile& traceFile) {
        if (read_trace_file_marker(traceFile) != 0) {
            write_to_log(
                LogLevel::Fatal,
                "Missing swimps trace file marker."
            );

            return false;
        }

        return true;
    }


    EntryKind read_next_entry_kind(TraceFile& traceFile) {
        char buffer[swimps_v1_trace_entry_marker_size];
        memset(buffer, 0, sizeof buffer);

        const auto readReturnCode = traceFile.read(buffer);

        format_and_write_to_log<64>(
            LogLevel::Debug,
            "Entry marker: %.",
            buffer
        );

        if (readReturnCode == 0) {
            return EntryKind::EndOfFile;
        }

        if (readReturnCode != swimps_v1_trace_entry_marker_size) {
            return EntryKind::Unknown;
        }

        if (memcmp(buffer, swimps_v1_trace_file_marker, sizeof swimps_v1_trace_file_marker) == 0) {
            // Ignore this and grab the next one.
            return read_next_entry_kind(traceFile);
        }

        if (memcmp(buffer, swimps_v1_trace_sample_marker, sizeof swimps_v1_trace_sample_marker) == 0) {
            return EntryKind::Sample;
        }

        if (memcmp(buffer, swimps_v1_trace_symbolic_backtrace_marker, sizeof swimps_v1_trace_symbolic_backtrace_marker) == 0) {
            return EntryKind::SymbolicBacktrace;
        }

        if (memcmp(buffer, swimps_v1_trace_stack_frame_marker, sizeof swimps_v1_trace_stack_frame_marker) == 0) {
            return EntryKind::StackFrame;
        }

        return EntryKind::Unknown;
    }

    std::optional<Sample> read_sample(TraceFile& traceFile) {
        backtrace_id_t backtraceID;

        if (! traceFile.read(backtraceID)) {
            return {};
        }

        TimeSpecification timestamp;

        if (! traceFile.read(timestamp.seconds)) {
            return {};
        }

        if (! traceFile.read(timestamp.nanoseconds)) {
            return {};
        }

        return {{ backtraceID, timestamp }};
    }

    int write_trace_file_marker(TraceFile& targetFile) {
        const auto bytesWritten = targetFile.write(swimps_v1_trace_file_marker);

        if (bytesWritten != sizeof swimps_v1_trace_file_marker) {
            targetFile.remove();
            return -1;
        }

        return 0;
    }

    std::optional<Backtrace> read_backtrace(TraceFile& traceFile) {
        Backtrace backtrace;

        if (! traceFile.read(backtrace.id)) {
            return {};
        }

        if (! traceFile.read(backtrace.stackFrameIDCount)) {
            return {};
        }

        swimps_assert(backtrace.stackFrameIDCount > 0);

        for (stack_frame_count_t i = 0; i < backtrace.stackFrameIDCount; ++i) {
            if (! traceFile.read(backtrace.stackFrameIDs[i])) {
                return {};
            }
        }

        return backtrace;
    }

    std::optional<StackFrame> read_stack_frame(TraceFile& traceFile) {
        StackFrame stackFrame;

        if (! traceFile.read(stackFrame.id)) {
            return {};
        }

        if (! traceFile.read(stackFrame.mangledFunctionNameLength)) {
            return {};
        }

        {
            const auto bytesToWrite = std::min(
                static_cast<size_t>(stackFrame.mangledFunctionNameLength),
                sizeof StackFrame::mangledFunctionName
            );

            if (traceFile.read({
                    stackFrame.mangledFunctionName,
                    bytesToWrite
                }) != bytesToWrite) {
                return {};
            }
        }

        if (traceFile.read(stackFrame.offset) != sizeof(stackFrame.offset)) {
            return {};
        }

        return stackFrame;
    }
}

TraceFile TraceFile::create(const Span<const char> path, const Permissions permissions) noexcept {
    TraceFile traceFile;
    traceFile.create_internal(
        path,
        permissions
    );

    // Write out the swimps marker to make such files easily recognisable
    const auto writeMarkerReturnValue = write_trace_file_marker(traceFile);
    swimps_assert(writeMarkerReturnValue != -1);

    return traceFile;
}

TraceFile TraceFile::create_temporary(const Span<const char> pathPrefix, const Permissions permissions) noexcept {
    TraceFile traceFile;
    traceFile.create_temporary_internal(
        pathPrefix,
        permissions
    );

    // Write out the swimps marker to make such files easily recognisable
    const auto writeMarkerReturnValue = write_trace_file_marker(traceFile);
    swimps_assert(writeMarkerReturnValue != -1);

    return traceFile;
}

TraceFile TraceFile::open(const Span<const char> path, const Permissions permissions) noexcept {
    TraceFile traceFile;
    traceFile.open_internal(path, permissions);

    swimps_assert(isSwimpsTraceFile(traceFile));

    return traceFile;
}

std::size_t swimps::trace::file::TraceFile::add_backtrace(const Backtrace& backtrace) {
    std::size_t bytesWritten = 0;

    bytesWritten += write(swimps_v1_trace_symbolic_backtrace_marker);
    bytesWritten += write(backtrace.id);
    bytesWritten += write(backtrace.stackFrameIDCount);

    for(stack_frame_count_t i = 0; i < backtrace.stackFrameIDCount; ++i) {
        bytesWritten += write(backtrace.stackFrameIDs[i]);
    }

    return bytesWritten;
}

std::size_t TraceFile::add_stack_frame(const StackFrame& stackFrame) {
    std::size_t bytesWritten = 0;

    bytesWritten += write(swimps_v1_trace_stack_frame_marker);

    const auto  id = stackFrame.id;
    const auto& mangledFunctionName = stackFrame.mangledFunctionName;
    const auto  mangledFunctionNameLength = static_cast<mangled_function_name_length_t>(strnlen(&mangledFunctionName[0], sizeof mangledFunctionName));

    const auto& offset = stackFrame.offset;

    swimps_assert(mangledFunctionNameLength >= 0);

    bytesWritten += write(id);
    bytesWritten += write(mangledFunctionNameLength);
    bytesWritten += write({ &mangledFunctionName[0], static_cast<size_t>(mangledFunctionNameLength) });
    bytesWritten += write(offset);

    return bytesWritten;
}


std::size_t swimps::trace::file::TraceFile::add_sample(const Sample& sample) {
    std::size_t bytesWritten = 0;

    bytesWritten += write(swimps_v1_trace_sample_marker);
    bytesWritten += write(sample.backtraceID);
    bytesWritten += write(sample.timestamp.seconds);
    bytesWritten += write(sample.timestamp.nanoseconds);

    return bytesWritten;
}

TraceFile::Entry TraceFile::read_next_entry() noexcept {
    const auto entryKind = read_next_entry_kind(*this);

    format_and_write_to_log<128>(
        LogLevel::Debug,
        "Trace file entry kind: %.",
        static_cast<int>(entryKind)
    );

    switch(entryKind) {
    case EntryKind::Sample:
        {
            const auto sample = read_sample(*this);
            if (!sample) {

                write_to_log(
                    LogLevel::Fatal,
                    "Reading sample failed."
                );

                return ErrorCode::ReadSampleFailed;
            }

            return *sample;
        }
    case EntryKind::SymbolicBacktrace:
        {
            const auto backtrace = read_backtrace(*this);
            if (!backtrace) {
                write_to_log(
                    LogLevel::Fatal,
                    "Reading backtrace failed."
                );

                return ErrorCode::ReadBacktraceFailed;
            }

            return *backtrace;
        }
    case EntryKind::StackFrame:
        {
            const auto stackFrame = read_stack_frame(*this);
            if (!stackFrame) {
                write_to_log(
                    LogLevel::Fatal,
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
        write_to_log(
            LogLevel::Debug,
            "Unknown entry kind detected, bailing."
        );

        return ErrorCode::UnknownEntryKind;
    }
}

bool swimps::trace::file::TraceFile::finalise() noexcept {
    if (! goToStartOfFile(*this)) {
        return false;
    }

    struct BacktraceHash {
        size_t operator() (const Backtrace& backtrace) const {
            swimps_assert(backtrace.stackFrameIDCount > 0);
            return std::hash<stack_frame_id_t>{}(backtrace.stackFrameIDs[0]);
        }
    };

    struct BacktraceEqual {
        bool operator() (
            const Backtrace& lhs,
            const Backtrace& rhs
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
        size_t operator() (const StackFrame& stackFrame) const {
            return std::hash<const char*>{}(stackFrame.mangledFunctionNameLength == 0 ? nullptr : &stackFrame.mangledFunctionName[0]);
        }
    };

    struct StackFrameEqual {
        bool operator() (
            const StackFrame& lhs,
            const StackFrame& rhs
        ) const {
            return lhs.isEquivalentTo(rhs);
        }
    };

    std::vector<Sample> samples;

    std::unordered_map<
        Backtrace,
        std::unordered_set<backtrace_id_t>,
        BacktraceHash,
        BacktraceEqual> backtraceMap;

    std::unordered_map<
        StackFrame,
        std::unordered_set<stack_frame_id_t>,
        StackFrameHash,
        StackFrameEqual> stackFrameMap;

    bool stop = false;
    size_t preOptimisationSampleCount = 0;
    size_t preOptimisationBacktraceCount = 0;
    size_t preOptimisationStackFrameCount = 0;

    for (auto entry = read_next_entry();
         !stop ;
         entry = read_next_entry()) {
        
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

    std::vector<Sample> samplesSharingBacktraceID;
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

    std::vector<Backtrace> backtracesSharingStackFrameID;
    for(const auto& [oldBacktrace, unused] : backtraceMap) {

        Backtrace newBacktrace;
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

    auto tempFile = TraceFile::create_temporary("swimps_finalise_temp_file_", TraceFile::Permissions::ReadWrite);;

    format_and_write_to_log<512>(
        LogLevel::Debug,
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
        tempFile.add_stack_frame(stackFrame.first);
    }

    const auto traceFilePath = getPath();
    const std::string traceFilePathString(&traceFilePath[0], traceFilePath.current_size());
    const auto tempFilePath = tempFile.getPath();
    const std::string tempFilePathString(&tempFilePath[0], tempFilePath.current_size());
    std::filesystem::copy(tempFilePathString, traceFilePathString, std::filesystem::copy_options::overwrite_existing);

    return true;
}

std::optional<Trace> swimps::trace::file::TraceFile::read_trace() noexcept {
    if (! goToStartOfFile(*this)) {
        return {};
    }

    Trace trace;

    bool stop = false;
    for (auto entry = read_next_entry();
         !stop ;
         entry = read_next_entry()) {

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

