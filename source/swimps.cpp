#include "swimps-profile.h"
#include "swimps-option-parser.h"
#include "swimps-log.h"
#include "swimps-analysis.h"
#include "swimps-trace-file.h"
#include "swimps-assert.h"

#include <functional>
#include <iostream>

#include <sys/stat.h>
#include <fcntl.h>

using CallTreeNode = swimps::analysis::Analysis::CallTreeNode;
using swimps::trace::TraceFile;

int main(int argc, char** argv) {
    bool exceptionThrown = false;
    swimps::option::Options options;

    try {
        options = swimps::option::parse_command_line(
            argc,
            const_cast<const char**>(argv)
        );
    } catch (const std::exception& exception) {
        swimps::log::format_and_write_to_log<2048>(
            swimps::log::LogLevel::Fatal,
            "Error encountered whilst parsing command line: %",
            exception.what()
        );

        exceptionThrown = true;
    }

    if (exceptionThrown || options.help) {
        swimps::option::print_help();
        exit(static_cast<int>(
            exceptionThrown
                ? swimps::error::ErrorCode::CommandLineParseFailed
                : swimps::error::ErrorCode::None
        ));
    }

    swimps::log::setLevelToLog(options.logLevel);
    const auto profileResult = swimps::profile::start(options);
    if (profileResult != swimps::error::ErrorCode::None) {
        swimps::log::format_and_write_to_log<256>(
            swimps::log::LogLevel::Fatal,
            "Profile failed with code: %",
            static_cast<int>(profileResult)
        );

        return static_cast<int>(profileResult);
    }

    auto traceFile = TraceFile::open(
        { options.targetTraceFile.c_str(), options.targetTraceFile.size() },
        swimps::io::File::Permissions::ReadOnly
    );

    const auto trace = traceFile.read_trace();
    const auto& stackFrames = trace->stackFrames;

    const auto analysis = swimps::analysis::analyse(*trace);

    for (const auto& entry : analysis.backtraceFrequency) {
        const auto sampleCount = entry.first;
        const auto backtraceID = entry.second;


        const auto& backtraces = trace->backtraces;
        const auto& backtraceIter = std::find_if(
            backtraces.cbegin(),
            backtraces.cend(),
            [backtraceID](const auto& backtrace){ return backtrace.id == backtraceID; }
        );

        swimps_assert(backtraceIter != backtraces.cend());

        const auto& backtrace = *backtraceIter;

        std::cout << "Backtrace #" << backtraceID << " (" << sampleCount << " times):\n";
        for (swimps::trace::stack_frame_count_t i = 0; i < backtrace.stackFrameIDCount; ++i) {
            const auto& stackFramesIter = std::find_if(
                stackFrames.cbegin(),
                stackFrames.cend(),
                [backtrace, i](const auto& stackFrame){ return backtrace.stackFrameIDs[i] == stackFrame.id; }
            );

            swimps_assert(stackFramesIter != stackFrames.cend());

            std::cout << "    Frame #" << i << ": " << stackFramesIter->mangledFunctionName << "+" << stackFramesIter->offset << "\n";
        }

        std::cout << std::endl;
    }

    {
        std::function<void(const CallTreeNode&, std::size_t)> printNode;
        printNode = [&printNode, &stackFrames](const CallTreeNode& node, const std::size_t desiredIndent){
            for (std::size_t currentIndent = 0; currentIndent < desiredIndent; ++currentIndent) {
                std::cout << "-";
            }

            std::cout << "> ";

            const auto stackFrameIter = std::find_if(
                stackFrames.cbegin(),
                stackFrames.cend(),
                [&node](const auto& stackFrame) { return stackFrame.id == node.stackFrameID; }
            );

            swimps_assert(stackFrameIter != stackFrames.cend());

            std::cout << stackFrameIter->mangledFunctionName << "+" << stackFrameIter->offset << " (" << node.frequency << " hits in this branch)" << std::endl;
            for (const auto& child : node.children) {
                printNode(child, desiredIndent + 1);
            }
        };

        for (const auto& root : analysis.callTree) {
            printNode(root, 0);
            std::cout << "#################################################################" << std::endl;
        }
    }

    return static_cast<int>(swimps::error::ErrorCode::None);
}
