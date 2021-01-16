#include "swimps-profile.h"
#include "swimps-option-parser.h"
#include "swimps-log.h"
#include "swimps-analysis.h"
#include "swimps-trace-file.h"
#include "swimps-tui.h"
#include "swimps-assert.h"

#include <functional>
#include <iostream>

#include <sys/stat.h>
#include <fcntl.h>

using CallTreeNode = swimps::analysis::Analysis::CallTreeNode;
using swimps::error::ErrorCode;
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
    const auto analysis = swimps::analysis::analyse(*trace);

    if (options.tui) {
        return static_cast<int>(swimps::tui::run(*trace, analysis));
    }

    return static_cast<int>(ErrorCode::None);
}
