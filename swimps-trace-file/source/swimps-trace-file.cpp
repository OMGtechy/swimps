#include "swimps-trace-file/swimps-trace-file.h"

#include <cstring>
#include <cerrno>
#include <cinttypes>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <string>
#include <filesystem>
#include <fstream>

#include <fcntl.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include <samplerpreload/trace-file.hpp>

#include <signalsafe/memory.hpp>

#include "swimps-assert/swimps-assert.h"
#include "swimps-log/swimps-log.h"

using signalsafe::memory::copy_no_overlap;
using signalsafe::time::TimeSpecification;

using signalsampler::instruction_pointer_t;

using swimps::error::ErrorCode;
using swimps::log::format_and_write_to_log;
using swimps::log::LogLevel;
using swimps::log::write_to_log;
using swimps::trace::Backtrace;
using swimps::trace::backtrace_id_t;
using swimps::trace::function_name_length_t;
using swimps::trace::ProcMaps;
using swimps::trace::RawSample;
using swimps::trace::Sample;
using swimps::trace::StackFrame;
using swimps::trace::stack_frame_count_t;
using swimps::trace::stack_frame_id_t;
using swimps::trace::Trace;
using swimps::trace::TraceFile;

namespace {
    constexpr size_t swimps_v1_trace_entry_marker_size = 6;
    constexpr char swimps_v1_trace_file_marker[swimps_v1_trace_entry_marker_size] = "s_v1\n";
    constexpr char swimps_v1_trace_proc_maps_marker[swimps_v1_trace_entry_marker_size] = "\npm!\n";
    constexpr char swimps_v1_trace_symbolic_backtrace_marker[swimps_v1_trace_entry_marker_size] = "\nsb!\n";
    constexpr char swimps_v1_trace_raw_sample_marker[swimps_v1_trace_entry_marker_size] = "\nrs!\n";
    constexpr char swimps_v1_trace_sample_marker[swimps_v1_trace_entry_marker_size] = "\nsp!\n";
    constexpr char swimps_v1_trace_stack_frame_marker[swimps_v1_trace_entry_marker_size] = "\nsf!\n";

    struct Visitor {
        using BacktraceHandler = std::function<void(Backtrace&)>;
        using SampleHandler = std::function<void(Sample&)>;
        using RawSampleHandler = std::function<void(RawSample&)>;
        using StackFrameHandler = std::function<void(StackFrame&)>;
        using ProcMapsHandler = std::function<void(ProcMaps&)>;

        Visitor(bool& stopTarget, BacktraceHandler onBacktrace, SampleHandler onSample, RawSampleHandler onRawSample, StackFrameHandler onStackFrame, ProcMapsHandler onProcMaps)
        : m_stopTarget(stopTarget), m_onBacktrace(onBacktrace), m_onSample(onSample), m_onRawSample(onRawSample), m_onStackFrame(onStackFrame), m_onProcMaps(onProcMaps) {

        }

        bool& m_stopTarget;
        BacktraceHandler m_onBacktrace;
        SampleHandler m_onSample;
        RawSampleHandler m_onRawSample;
        StackFrameHandler m_onStackFrame;
        ProcMapsHandler m_onProcMaps;

        void operator()(ProcMaps& procMaps) const {
            m_onProcMaps(procMaps);
        }

        void operator()(Sample& sample) const {
            m_onSample(sample);
        }

        void operator()(RawSample& rawSample) const {
            m_onRawSample(rawSample);
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
        ProcMaps,
        Sample,
        RawSample,
        SymbolicBacktrace,
        StackFrame,
    };

    int read_trace_file_marker(TraceFile& traceFile) {
        std::array<std::byte, sizeof swimps_v1_trace_file_marker> buffer;
        if (! traceFile.read(buffer)) {
            return -1;
        }

        return memcmp(buffer.data(), swimps_v1_trace_file_marker, sizeof swimps_v1_trace_file_marker) == 0 ? 0 : -1;
    }

    bool goToStartOfFile(TraceFile& file) {
        if (file.seek(0, TraceFile::OffsetInterpretation::Absolute) != 0) {
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

        if (memcmp(buffer, swimps_v1_trace_proc_maps_marker, sizeof swimps_v1_trace_proc_maps_marker) == 0) {
            return EntryKind::ProcMaps;
        }

        if (memcmp(buffer, swimps_v1_trace_raw_sample_marker, sizeof swimps_v1_trace_raw_sample_marker) == 0) {
            return EntryKind::RawSample;
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

    std::optional<ProcMaps> read_proc_maps(TraceFile& traceFile) {
        ProcMaps::entry_count_t entryCount;

        if (! traceFile.read(entryCount)) {
            return {};
        }

        ProcMaps procMaps;
        procMaps.entries.resize(entryCount);

        for (ProcMaps::entry_count_t i = 0; i < entryCount; ++i) {
            auto& entry = procMaps.entries[i];

            if (! traceFile.read(entry.range.start)) {
                return {};
            }

            if (! traceFile.read(entry.range.end)) {
                return {};
            }
        }

        return procMaps;
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

    std::optional<RawSample> read_raw_sample(TraceFile& traceFile) {
        decltype(RawSample::backtrace) backtrace;

        for (std::size_t i = 0;; ++i) {
            // We should hit an instruction pointer of 0 *before* this happens.
            swimps_assert(i < backtrace.size());

            if (! traceFile.read(backtrace[i])) {
                return {};
            }

            if (backtrace[i] == 0) {
                break;
            }
        }

        TimeSpecification timestamp;

        if (! traceFile.read(timestamp.seconds)) {
            return {};
        }

        if (! traceFile.read(timestamp.nanoseconds)) {
            return {};
        }

        return {{ backtrace, timestamp }};
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

        if (! traceFile.read(stackFrame.functionNameLength)) {
            return {};
        }

        {
            const auto bytesToWrite = std::min(
                static_cast<size_t>(stackFrame.functionNameLength),
                sizeof StackFrame::functionName
            );

            if (traceFile.read({
                    stackFrame.functionName,
                    bytesToWrite
                }) != bytesToWrite) {
                return {};
            }
        }

        if (traceFile.read(stackFrame.offset) != sizeof(stackFrame.offset)) {
            return {};
        }

        if (traceFile.read(stackFrame.instructionPointer) != sizeof(stackFrame.instructionPointer)) {
            return {};
        }

        if (traceFile.read(stackFrame.lineNumber) != sizeof(stackFrame.lineNumber)) {
            return {};
        }

        if (traceFile.read(stackFrame.sourceFilePathLength) != sizeof(stackFrame.sourceFilePathLength)) {
            return {};
        }

        if (traceFile.read({ stackFrame.sourceFilePath,
                             stackFrame.sourceFilePathLength }) != stackFrame.sourceFilePathLength) {
            return {};
        }

        return stackFrame;
    }
}

TraceFile TraceFile::create_and_open(std::string_view path, const Permissions permissions) noexcept {
    format_and_write_to_log<128>(
        LogLevel::Debug,
        "%: creating %",
        __func__,
        path.data()
    );

    TraceFile traceFile;
    traceFile.create_and_open_internal(
        path,
        permissions
    );

    // Write out the swimps marker to make such files easily recognisable
    const auto writeMarkerReturnValue = write_trace_file_marker(traceFile);
    swimps_assert(writeMarkerReturnValue != -1);

    return traceFile;
}

TraceFile TraceFile::create_temporary() noexcept {
    TraceFile traceFile;
    traceFile.create_and_open_temporary_internal();

    // Write out the swimps marker to make such files easily recognisable
    const auto writeMarkerReturnValue = write_trace_file_marker(traceFile);
    swimps_assert(writeMarkerReturnValue != -1);

    return traceFile;
}

TraceFile TraceFile::open_existing(std::string_view path, const Permissions permissions) noexcept {
    format_and_write_to_log<128>(
        LogLevel::Debug,
        "%: opening %",
        __func__,
        path.data()
    );

    TraceFile traceFile;
    traceFile.open_existing_internal(path, permissions);

    swimps_assert(isSwimpsTraceFile(traceFile));

    return traceFile;
}

TraceFile TraceFile::from_raw(std::string_view pathView) noexcept {
    // TODO: now TraceFile doesn't need to be signal-safe anymore, why not pass in std::filesystem::paths directly?

    std::filesystem::path path(pathView);
    swimps_assert(std::filesystem::exists(path));

    std::ifstream rawFile(path.native(), std::ios_base::in | std::ios_base::binary);
    swimps_assert(rawFile.is_open());

    std::noskipws(rawFile);

    std::vector<unsigned char> rawData(std::filesystem::file_size(path));

    rawFile.read(reinterpret_cast<char*>(rawData.data()), rawData.size());
    rawFile.close();

    const auto rawTrace = samplerpreload::Trace::from(rawData);
    std::filesystem::remove(path);

    auto traceFile = create_and_open(path.string(), Permissions::ReadWrite);
    for (const auto& sample : rawTrace.get_samples()) {
        RawSample rawSample;
        const auto& backtrace = sample.backtrace;
        std::copy_n(backtrace.cbegin(), std::min(backtrace.size(), backtrace.size()), rawSample.backtrace.begin());
        rawSample.timestamp = sample.timestamp;

        traceFile.add_raw_sample(rawSample);
    }

    ProcMaps procMaps;
    std::cout << "proc map pre size: " << rawTrace.get_proc_maps().ranges.size() << std::endl;
    for (auto range : rawTrace.get_proc_maps().ranges) {
        procMaps.entries.push_back({range.start, range.end});
    }

    traceFile.set_proc_maps(procMaps);

    return traceFile;
}

std::size_t TraceFile::set_proc_maps(const ProcMaps& procMaps) {
    std::size_t bytesWritten = 0;

    const auto entryCount = static_cast<ProcMaps::entry_count_t>(procMaps.entries.size());

    bytesWritten += write(swimps_v1_trace_proc_maps_marker);    
    bytesWritten += write(entryCount);

    for(const auto& entry : procMaps.entries) {
        bytesWritten += write(entry.range.start);
        bytesWritten += write(entry.range.end);
    };

    return bytesWritten;
}

std::size_t TraceFile::add_backtrace(const Backtrace& backtrace) {
    std::size_t bytesWritten = 0;

    swimps_assert(backtrace.stackFrameIDCount >0);

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
    const auto& functionName = stackFrame.functionName;
    const auto  functionNameLength = static_cast<function_name_length_t>(strnlen(&functionName[0], sizeof functionName));

    const auto& offset = stackFrame.offset;
    const auto& instructionPointer = stackFrame.instructionPointer;

    const auto& lineNumber = stackFrame.lineNumber;
    const auto& sourceFilePath = stackFrame.sourceFilePath;
    const auto  sourceFilePathLength = stackFrame.sourceFilePathLength;

    swimps_assert(functionNameLength >= 0);

    bytesWritten += write(id);
    bytesWritten += write(functionNameLength);
    bytesWritten += write({ &functionName[0], static_cast<size_t>(functionNameLength) });
    bytesWritten += write(offset);
    bytesWritten += write(instructionPointer);
    bytesWritten += write(lineNumber);
    bytesWritten += write(sourceFilePathLength);
    bytesWritten += write({ sourceFilePath, static_cast<size_t>(sourceFilePathLength) });

    return bytesWritten;
}

std::size_t TraceFile::add_sample(const Sample& sample) {
    std::size_t bytesWritten = 0;

    bytesWritten += write(swimps_v1_trace_sample_marker);
    bytesWritten += write(sample.backtraceID);
    bytesWritten += write(sample.timestamp.seconds);
    bytesWritten += write(sample.timestamp.nanoseconds);

    return bytesWritten;
}

std::size_t TraceFile::add_raw_sample(const RawSample& rawSample) {
    std::size_t bytesWritten = 0;

    bytesWritten += write(swimps_v1_trace_raw_sample_marker);

    for (std::size_t i = 0; rawSample.backtrace[i] != 0; ++i) {
        bytesWritten += write(rawSample.backtrace[i]);
    }

    bytesWritten += write(instruction_pointer_t{0});
    bytesWritten += write(rawSample.timestamp.seconds);
    bytesWritten += write(rawSample.timestamp.nanoseconds);

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
    case EntryKind::ProcMaps:
        {
            const auto procMaps = read_proc_maps(*this);
            if (! procMaps) {
                write_to_log(
                    LogLevel::Fatal,
                    "Reading proc maps failed."
                );

                return ErrorCode::ReadProcMapsFailed;
            }

            return *procMaps;
        }
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
    case EntryKind::RawSample:
        {
            const auto rawSample = read_raw_sample(*this);
            if (!rawSample) {

                write_to_log(
                    LogLevel::Fatal,
                    "Reading raw sample failed."
                );

                return ErrorCode::ReadRawSampleFailed;
            }

            return *rawSample;
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

bool TraceFile::finalise() noexcept {
    if (! goToStartOfFile(*this)) {
        return false;
    }

    ProcMaps procMaps;

    stack_frame_id_t nextStackFrameID = 1;
    std::unordered_map<instruction_pointer_t, stack_frame_id_t> stackFrameIDMap;

    struct BacktraceHash final {
        std::size_t operator()(const Backtrace& backtrace) const {
            return backtrace.id;
        }
    };

    struct BacktraceCompare final {
        bool operator()(const Backtrace& lhs, const Backtrace& rhs) const {
            // TODO: don't check beyond stackFrameIDCount?
            return lhs.stackFrameIDs == rhs.stackFrameIDs;
        }
    };

    backtrace_id_t nextBacktraceID = 1;
    std::unordered_map<Backtrace, backtrace_id_t, BacktraceHash, BacktraceCompare> backtraces;
    std::vector<StackFrame> stackFrames;
    std::vector<Sample> samples;

    // Grab a 0.5GB of RAM for each.
    constexpr std::size_t halfAGigInBytes = 500'000'000;
    samples.reserve(halfAGigInBytes / sizeof(Sample));
    stackFrames.reserve(halfAGigInBytes / sizeof(StackFrame));

    bool stop = false;
    for (auto entry = read_next_entry(); !stop; entry = read_next_entry()) {
        std::visit(
            // TODO: add visitor for unfainlised so the dummy lambdas aren't needed?
            Visitor{
                stop,
                // TODO: add errors for unexpected entries
                [](auto& /* backtrace */){},
                [](auto& /* sample */){},
                [&stackFrameIDMap, &samples, &stackFrames, &backtraces, &nextBacktraceID, &nextStackFrameID](auto& rawSample){
                    Backtrace backtrace;

                    swimps_assert(backtrace.stackFrameIDs.size() == rawSample.backtrace.size());

                    for (std::size_t i = 0; i < rawSample.backtrace.size() && rawSample.backtrace[i] != 0; ++i) {

                        const auto instructionPointer = rawSample.backtrace[i];

                        if (stackFrameIDMap.find(instructionPointer) == stackFrameIDMap.cend()) {
                            stackFrameIDMap[instructionPointer] = nextStackFrameID++;
                        }

                        const auto stackFrameID = stackFrameIDMap.at(instructionPointer);

                        backtrace.stackFrameIDs[i] = stackFrameIDMap.at(instructionPointer);
                        backtrace.stackFrameIDCount += 1;

                        stackFrames.emplace_back(stackFrameID, instructionPointer);
                    }

                    auto backtraceIter = backtraces.find(backtrace);
                    if (backtraceIter == backtraces.cend()) {
                        backtraceIter = backtraces.insert({backtrace, nextBacktraceID++}).first;
                    }

                    samples.push_back({backtraceIter->second, rawSample.timestamp});
                },
                [](auto& /* stackFrame */){},
                [&procMaps](auto& _procMaps) {
                    procMaps = _procMaps;
                }
            },
            entry
        );
    }

    format_and_write_to_log<1024>(
        LogLevel::Debug,
        "Finalising...\n"
        "Samples: %\n"
        "Backtraces: %\n"
        "Stack Frames: %\n",
        samples.size(),
        backtraces.size(),
        stackFrames.size()
    );

    const auto traceFilePath = get_path();
    const auto tempFilePath = std::string("/tmp/") + std::filesystem::path(traceFilePath).filename().string() + ".tmp";

    auto tempFile = TraceFile::create_and_open({ tempFilePath.data(), tempFilePath.length() }, TraceFile::Permissions::ReadWrite);

    tempFile.set_proc_maps(procMaps);

    for(const auto& sample : samples) {
        tempFile.add_sample(sample);
    }

    for(const auto& backtraceKeyValue : backtraces) {
        tempFile.add_backtrace(backtraceKeyValue.first);
    }

    for(auto& stackFrame : stackFrames) {
        const auto instructionPointer = stackFrame.instructionPointer;

        unw_context_t unwindContext{};

        #ifdef __clang__
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wgnu-statement-expression"
        #endif
        unw_getcontext(&unwindContext);
        #ifdef __clang__
        #pragma clang diagnostic pop
        #endif
        unw_cursor_t unwindCursor{};
        unw_init_local(&unwindCursor, &unwindContext);
        unw_set_reg(&unwindCursor, UNW_REG_IP, instructionPointer);
        unw_get_proc_name(&unwindCursor, &stackFrame.functionName[0], std::size(stackFrame.functionName), &stackFrame.offset);

        tempFile.add_stack_frame(stackFrame);
    }

    std::filesystem::copy(tempFilePath, traceFilePath, std::filesystem::copy_options::overwrite_existing);

    return true;
}

std::optional<Trace> TraceFile::read_trace() noexcept {
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
                [](auto&){ swimps_assert(false); }, // shouldn't be getting raw samples as this point
                [&trace](auto& stackFrame){ trace.stackFrames.push_back(stackFrame); },
                [&trace](auto& procMaps){ trace.procMaps = procMaps; }
            },
            entry
        );
    }

    return trace;
}

