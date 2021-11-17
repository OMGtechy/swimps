#include "swimps-option/swimps-option-parser.h"

#include <iostream>
#include <filesystem>
#include <functional>
#include <string>

#include <signalsafe/string.hpp>
#include <signalsafe/time.hpp>

#include <CLI/CLI.hpp>

#include "swimps-assert/swimps-assert.h"
#include "swimps-error/swimps-error.h"
#include "swimps-trace-file/swimps-trace-file.h"

using signalsafe::string::format;
using signalsafe::time::now;
using signalsafe::time::TimeSpecification;

using namespace swimps::option;

using swimps::error::ErrorCode;
using swimps::log::LogLevel;

inline static std::ostream &operator<<(std::ostream &os, const LogLevel logLevel) {
    switch(logLevel) {
    case LogLevel::Fatal:   os << "Fatal";   break;
    case LogLevel::Error:   os << "Error";   break;
    case LogLevel::Warning: os << "Warning"; break;
    case LogLevel::Info:    os << "Info";    break;
    case LogLevel::Debug:   os << "Debug";   break;
    default:                os << "Unknown"; break;
    }

    return os;
}

std::optional<Options> swimps::option::parse_command_line(
    int argc,
    const char* argv[]) {

    swimps_assert(argv != nullptr);

    Options options;
    CLI::App cliApp;

    cliApp.add_flag("--load", options.load, "Load the target trace file rather than creating a new one.");
    cliApp.add_flag("--tui,!--no-tui", options.tui, "Toggle the TUI.");
    cliApp.add_flag("--ptrace,!--no-ptrace", options.ptrace, "Toggle ptrace."); 
    cliApp.add_option("--target-trace-file", options.targetTraceFile);
    cliApp.add_option("--samples-per-second", options.samplesPerSecond);

    const auto logLevelMap = std::map<std::string, LogLevel>{
        {"debug",   LogLevel::Debug},
        {"info",    LogLevel::Info},
        {"warning", LogLevel::Warning},
        {"error",   LogLevel::Error},
        {"fatal",   LogLevel::Fatal}
    };

    cliApp.add_option("--log-level", options.logLevel, "The verbosity of log messages.")
        ->transform(CLI::CheckedTransformer(logLevelMap)
            .description("{debug, info, warning, error, fatal}"));

    cliApp.prefix_command();

    [&cliApp, &argc, &argv](){ CLI11_PARSE(cliApp, argc, argv); return 0; }();

    if (cliApp.get_help_ptr()->operator bool()) {
        exit(0);
    }

    const auto remaining = cliApp.remaining(true);

    if (options.load) {
        return options;
    }

    if (remaining.size() == 0) {
        cliApp.exit({"No target program specified.", "Please specify a target program."});
        return {};
    }

    options.targetProgram = remaining.at(0);
    std::copy(remaining.cbegin() + 1, remaining.cend(), std::back_inserter(options.targetProgramArgs));

    if (options.targetTraceFile.empty()) {
        const auto time = now(CLOCK_MONOTONIC);
        char targetTraceFileBuffer[1024] = { };
        format(
            "swimps_trace_%_%_%",
            targetTraceFileBuffer,
            std::filesystem::path(options.targetProgram).filename().c_str(),
            time.seconds,
            time.nanoseconds
        );

        options.targetTraceFile = targetTraceFileBuffer;
    }

    return options;
}

