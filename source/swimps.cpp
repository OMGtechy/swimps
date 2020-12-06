#include "swimps-profile.h"
#include "swimps-option-parser.h"
#include "swimps-log.h"
#include "swimps-analysis.h"
#include "swimps-trace-file.h"

#include <iostream>

#include <sys/stat.h>
#include <fcntl.h>

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
            "Error encountered whilst parsing command line: %s",
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
            "Profile failed with code: %d",
            static_cast<int>(profileResult)
        );

        return static_cast<int>(profileResult);
    }

    const auto targetTraceFileDescriptor = open(options.targetTraceFile.c_str(), O_RDONLY);
    if (targetTraceFileDescriptor == -1) {
        swimps::log::format_and_write_to_log<256>(
            swimps::log::LogLevel::Fatal,
            "Failed to open trace file: %s",
            options.targetTraceFile.c_str()
        );

        return static_cast<int>(swimps::error::ErrorCode::OpenFailed);
    }

    const auto trace = swimps::trace::file::read(targetTraceFileDescriptor);
    const auto analysis = swimps::analysis::analyse(*trace);

    for (auto& entry : analysis.backtraceFrequency) {
        const auto& backtrace = trace->backtraces[entry.first];

        std::cout << "Backtrace #" << entry.first << " (" << entry.second << " times):\n";
        for (swimps::trace::stack_frame_count_t i = 0; i < backtrace.stackFrameCount; ++i) {
            std::cout << "    Frame #" << i << ": " << backtrace.stackFrames[i].mangledFunctionName << "\n";
        }
        std::cout << std::endl;
    }

    return static_cast<int>(swimps::error::ErrorCode::None);
}
