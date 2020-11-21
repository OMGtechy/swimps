#include "swimps-option.h"

#include <iostream>

#include "swimps-assert.h"
#include "swimps-error.h"

using namespace swimps::option;
using swimps::log::LogLevel;
using swimps::error::ErrorCode;

namespace {
    constexpr char helpOptionName[] = "--help";

    constexpr char logLevelOptionName[] = "--log-level";
    LogLevel parseLogLevel(const std::string& currentArg, int& argc, const char**& argv) {
        swimps_assert(argv != nullptr);
        swimps_assert(currentArg == logLevelOptionName);
        swimps_assert(currentArg == std::string(*argv));

        if (argc < 2) {
            // We need one for the parameter name and one for the parameter value.
            throw ParseException(
                std::string("Missing value after ") + logLevelOptionName
            );
        }

        --argc;
        ++argv;

        swimps_assert(argc >= 1);
        swimps_assert(*argv != nullptr);

        const auto logLevelString = std::string(*argv);

        --argc;
        ++argv;

        if (logLevelString == "debug")        { return LogLevel::Debug; }
        else if (logLevelString == "info")    { return LogLevel::Info; }
        else if (logLevelString == "warning") { return LogLevel::Warning; }
        else if (logLevelString == "error")   { return LogLevel::Error; }
        else if (logLevelString == "fatal")   { return LogLevel::Fatal; }

        throw InvalidOptionValueException(
            logLevelOptionName,
            logLevelString,
            "debug, info, warning, error, fatal"
        );
    }
}

Options swimps::option::parse_command_line(
    int argc,
    const char* argv[]) {

    swimps_assert(argv != nullptr);

    // Skip the first, which should be the path to the swimps binary.
    --argc;
    ++argv;

    swimps::option::Options options;

    while(argc > 0) {
        swimps_assert(argv != nullptr);
        const auto currentArg = std::string(*argv);

        if (currentArg == logLevelOptionName) {
            options.logLevel = parseLogLevel(currentArg, argc, argv);
            continue;
        }

        if (currentArg == helpOptionName) {
            options.help = true;
            --argc;
            ++argv;
            continue;
        }

        if (currentArg.compare(0, 1, "-") == 0) {
            // This helps catch trailing args that haven't been processed.
            throw ParseException(std::string("Invalid option: ") + currentArg);
        }

        // Assume anything that's left is the program to be profiled, plus its args.

        swimps_assert(*argv != nullptr);
        options.targetProgram = currentArg;
        --argc;
        ++argv;

        while(argc > 0) {
            options.targetProgramArgs.push_back(*argv);
            --argc;
            ++argv;
        }
    }

    return options;
}

void swimps::option::print_help() {
    std::cout << "swimps: an open-source performance analysis tool.\n"
              << "\n"
              << "  Usage:   swimps [options]          [program]   [program arguments]\n"
              << "  Example: swimps --log--level debug ./myprogram --program-specific-argument\n"
              << "\n"
              << "  Options:\n"
              << "\n"
              << "    --log-level [debug,   The minimum severity required to show a low message.\n"
              << "                 info,\n"
              << "                 warning,\n"
              << "                 error,\n"
              << "                 fatal]\n"
              << "\n"
              << "    --help                Shows this help message.\n"
              << std::endl;
}
