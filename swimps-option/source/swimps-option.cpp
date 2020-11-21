#include "swimps-option.h"

#include <iostream>

#include "swimps-error.h"

using namespace swimps::option;

Options swimps::option::parse_command_line(
    int argc,
    char* argv[]) {

    cxxopts::Options options(
        "swimps",
        "swimps: an open-source performance analysis tool."
    );

    constexpr char helpOptionName[] = "help";
    constexpr char targetProgramOptionName[] = "target-program";
    constexpr char logLevelOptionName[] = "log-level";

    options.add_options()
        (helpOptionName, "Prints swimps command line help.")
        (targetProgramOptionName, "The program to profile.", cxxopts::value<std::string>())
        (logLevelOptionName, "Set how much information is logged [debug, info, warning, error, fatal].", cxxopts::value<std::string>()->default_value("info"));

    using swimps::log::LogLevel;

    const auto printHelp = [&options](){
        std::cout << options.help() << std::endl;
    };

    try {
        const auto parseResult = options.parse(argc, argv);

        if (parseResult[helpOptionName].as<bool>()) {
            printHelp();
            exit(static_cast<int>(swimps::error::ErrorCode::None));
        }

        swimps::option::Options result;

        result.targetProgram = parseResult[targetProgramOptionName].as<std::string>();

        const auto logLevelString = parseResult[logLevelOptionName].as<std::string>();
        if (logLevelString == "debug") { result.logLevel = LogLevel::Debug; }
        else if (logLevelString == "info") { result.logLevel = LogLevel::Info; }
        else if (logLevelString == "warning") { result.logLevel = LogLevel::Warning; }
        else if (logLevelString == "error") { result.logLevel = LogLevel::Error; }
        else if (logLevelString == "fatal") { result.logLevel = LogLevel::Fatal; }
        else {
            swimps::log::format_and_write_to_log<1024>(
                LogLevel::Fatal,
                "Unknown log level: %s",
                logLevelString.c_str()
            );

            exit(static_cast<int>(swimps::error::ErrorCode::CommandLineParseFailed));
        }

        return result;
    } catch (const std::exception& exception) {
        swimps::log::format_and_write_to_log<1024>(
            LogLevel::Fatal,
            "Failed to parse command line: %s",
            exception.what()
        );

        printHelp();
        exit(static_cast<int>(swimps::error::ErrorCode::CommandLineParseFailed));
    }
}
