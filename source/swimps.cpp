#include "swimps-profile/swimps-profile.h"
#include "swimps-option/swimps-option-parser.h"
#include "swimps-log/swimps-log.h"
#include "swimps-analysis/swimps-analysis.h"
#include "swimps-trace-file/swimps-trace-file.h"
#include "swimps-tui/swimps-tui.h"
#include "swimps-assert/swimps-assert.h"

#include <functional>
#include <iostream>

#include <sys/stat.h>
#include <fcntl.h>

using signalsafe::File;

using CallTreeNode = swimps::analysis::Analysis::CallTreeNode;
using swimps::error::ErrorCode;
using swimps::trace::TraceFile;

int main(int argc, char** argv) {
    auto maybeOptions = swimps::option::parse_command_line(
        argc,
        const_cast<const char**>(argv)
    );

    if (! maybeOptions.has_value()) {
        return static_cast<int>(swimps::error::ErrorCode::CommandLineParseFailed);
    }

    const auto options = *maybeOptions;

    swimps::log::setLevelToLog(options.logLevel);
    if (! options.load) {
        const auto profileResult = swimps::profile::start(options);
        if (profileResult != swimps::error::ErrorCode::None) {
            swimps::log::format_and_write_to_log<256>(
                swimps::log::LogLevel::Fatal,
                "Profile failed with code: %",
                static_cast<int>(profileResult)
            );

            return static_cast<int>(profileResult);
        }

        TraceFile::open_existing(
            { options.targetTraceFile.c_str(), options.targetTraceFile.size() },
            TraceFile::Permissions::ReadOnly
        ).finalise();
    }

    auto traceFile = TraceFile::open_existing(
        { options.targetTraceFile.c_str(), options.targetTraceFile.size() },
        TraceFile::Permissions::ReadOnly
    );

    const auto trace = traceFile.read_trace();
    const auto analysis = swimps::analysis::analyse(*trace);

    if (options.tui) {
        return static_cast<int>(swimps::tui::run(*trace, analysis));
    }

    return static_cast<int>(ErrorCode::None);
}
