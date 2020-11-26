#include "swimps-option.h"

#include <iostream>
#include <filesystem>
#include <sstream>
#include <string>

#include "swimps-assert.h"
#include "swimps-error.h"
#include "swimps-time.h"
#include "swimps-trace-file.h"

using namespace swimps::option;
using swimps::log::LogLevel;
using swimps::error::ErrorCode;

namespace {
    constexpr char helpOptionName[] = "--help";

    constexpr char targetTraceFileOptionName[] = "--target-trace-file";
    std::string parseTargetTraceFile(const std::string& currentArg, int& argc, const char**& argv) {
        swimps_assert(argv != nullptr);
        swimps_assert(currentArg == targetTraceFileOptionName);
        swimps_assert(currentArg == std::string(*argv));

        if (argc < 2) {
            // We need one for the parameter name and one for the parameter value.
            throw ParseException(
                std::string("Missing value after ") + targetTraceFileOptionName
            );
        }

        --argc;
        ++argv;

        swimps_assert(argc >= 1);
        swimps_assert(*argv != nullptr);

        const auto targetTraceFile = std::string(*argv);

        --argc;
        ++argv;

        return targetTraceFile;
    }

    constexpr char samplesPerSecondOptionName[] = "--samples-per-second";
    double parseSamplesPerSecond(const std::string& currentArg, int& argc, const char**& argv) {
        // TODO: this has a lot in common with parseLogLevel ... DRY?
        swimps_assert(argv != nullptr);
        swimps_assert(currentArg == samplesPerSecondOptionName);
        swimps_assert(currentArg == std::string(*argv));

        if (argc < 2) {
            // We need one for the parameter name and one for the parameter value.
            throw ParseException(
                std::string("Missing value after ") + samplesPerSecondOptionName
            );
        }

        --argc;
        ++argv;

        swimps_assert(argc >= 1);
        swimps_assert(*argv != nullptr);

        const auto samplesPerSecondString = std::string(*argv);

        --argc;
        ++argv;

        const auto samplesPerSecond = std::stod(samplesPerSecondString);

        if (samplesPerSecond < 0 || samplesPerSecond > 1'000'000'000 /* every nanosecond */) {
            throw InvalidOptionValueException(
                samplesPerSecondOptionName,
                samplesPerSecondString,
                "between 0 and 1,000,000,000, inclusive"
            );
        }

        return samplesPerSecond;
    }

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

// TODO: Options could probably do with its own file.

namespace {
    const std::string stringOptionsHelpLabel = "help ";
    const std::string stringOptionsLogLevelLabel = "log-level ";
    const std::string stringOptionsSamplesPerSecondLabel = "samples-per-second ";
    const std::string stringOptionsTargetTraceFileLabel = "target-trace-file ";
    const std::string stringOptionsTargetProgramLabel = "target-program ";
    const std::string stringOptionsTargetProgramArgsLabel = "target-program-args ";

    std::string chompPrefix(std::string string, std::string prefix) {
        swimps_assert(string.starts_with(prefix));
        return string.substr(prefix.size());
    }
}

Options swimps::option::Options::fromString(std::string string) {
    Options result;

    // help
    string = chompPrefix(string, stringOptionsHelpLabel);
    swimps_assert(string.length() >= 1);
    result.help = string[0] == '1';
    string = chompPrefix(string.substr(1), "|");

    // log level
    string = chompPrefix(string, stringOptionsLogLevelLabel);
    swimps_assert(string.length() >= 1);
    switch (string[0]) {
    case 'd': result.logLevel = LogLevel::Debug;   break;
    case 'i': result.logLevel = LogLevel::Info;    break;
    case 'w': result.logLevel = LogLevel::Warning; break;
    case 'e': result.logLevel = LogLevel::Error;   break;
    case 'f': result.logLevel = LogLevel::Fatal;   break;
    default:
        swimps_assert(false);
    }

    string = chompPrefix(string.substr(1), "|");

    // samples per second
    string = chompPrefix(string, stringOptionsSamplesPerSecondLabel);
    {
        const auto end = string.find("|");
        result.samplesPerSecond = std::stod(string.substr(0, end));
        string = string.substr(end + 1);
    }

    // target trace file
    string = chompPrefix(string, stringOptionsTargetTraceFileLabel);
    {
        const auto end = string.find("|");
        result.targetTraceFile = string.substr(0, end);
        string = string.substr(end + 1);
    }

    // target program
    string = chompPrefix(string, stringOptionsTargetProgramLabel);
    {
        const auto end = string.find("|");
        result.targetProgram = string.substr(0, end);
        string = string.substr(end + 1);
    }

    // target program args
    string = chompPrefix(string, stringOptionsTargetProgramArgsLabel);
    while (string.length() > 0) {
        const auto end = string.find("|");

        swimps_assert(end != 0);

        const auto nextArg = string.substr(0, end);
        result.targetProgramArgs.push_back(nextArg);
        string = string.substr(end + 1);
    }

    return result;
}

std::string swimps::option::Options::toString() const {
    std::stringstream stringStream;

    // help
    stringStream << stringOptionsHelpLabel << (help ? "1" : "0") << "|";

    // log level
    stringStream << stringOptionsLogLevelLabel;

    switch (logLevel) {
    case LogLevel::Debug:   stringStream << "d"; break;
    case LogLevel::Info:    stringStream << "i"; break;
    case LogLevel::Warning: stringStream << "w"; break;
    case LogLevel::Error:   stringStream << "e"; break;
    case LogLevel::Fatal:   stringStream << "f"; break;
    default:
        swimps_assert(false);
    }

    stringStream << "|";

    // samples per second
    stringStream << stringOptionsSamplesPerSecondLabel << samplesPerSecond << "|";

    // target trace file
    stringStream << stringOptionsTargetTraceFileLabel << targetTraceFile << "|";

    // target program
    stringStream << stringOptionsTargetProgramLabel << targetProgram << "|";

    // target program args
    stringStream << stringOptionsTargetProgramArgsLabel;

    for (auto& arg : targetProgramArgs) {
        stringStream << arg << "|";
    }

    return stringStream.str();
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

        if (currentArg == samplesPerSecondOptionName) {
            options.samplesPerSecond = parseSamplesPerSecond(currentArg, argc, argv);
            continue;
        }

        if (currentArg == targetTraceFileOptionName) {
            options.targetTraceFile = parseTargetTraceFile(currentArg, argc, argv);
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

    if (options.targetTraceFile.empty()) {
        swimps::time::TimeSpecification time;
        if (swimps::time::now(CLOCK_MONOTONIC, time) == -1) {
            throw ParseException("Could not get time to generate default trace file name.");
        }

        char targetTraceFileBuffer[1024] = { };
        swimps::trace::file::generate_name(
            std::filesystem::path(options.targetProgram).filename().c_str(),
            time,
            targetTraceFileBuffer
        );

        options.targetTraceFile = targetTraceFileBuffer;
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
              << "    --samples-per-second  How many samples to take per second when profiling.\n"
              << "\n"
              << "    --target-trace-file   Where to write the trace data.\n"
              << "\n"
              << "    --help                Shows this help message.\n"
              << std::endl;
}
