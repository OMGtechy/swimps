#include "swimps-option-parser.h"

#include <iostream>
#include <filesystem>
#include <functional>
#include <string>

#include "swimps-assert.h"
#include "swimps-error.h"
#include "swimps-time.h"
#include "swimps-trace-file.h"

using namespace swimps::option;

using swimps::error::ErrorCode;
using swimps::io::format_string;
using swimps::log::LogLevel;

namespace {
    constexpr char helpOptionName[] = "--help";
    constexpr char noTUIOptionName[] = "--no-tui";
    constexpr char targetTraceFileOptionName[] = "--target-trace-file";
    constexpr char samplesPerSecondOptionName[] = "--samples-per-second";
    constexpr char logLevelOptionName[] = "--log-level";

    template <typename T>
    inline T parseArgumentWithValue(
        const std::string& argName,
        const std::string& currentArg,
        int& argc,
        const char**& argv,
        const std::function<T(const char*)>& valueConversionFunction) {

        swimps_assert(argv != nullptr);
        swimps_assert(currentArg == argName);
        swimps_assert(currentArg == std::string(*argv));

        if (argc < 2) {
            // We need one for the parameter name and one for the parameter value.
            throw ParseException(
                std::string("Missing value after ") + currentArg
            );
        }

        --argc;
        ++argv;

        swimps_assert(argc >= 1);
        swimps_assert(*argv != nullptr);

        const auto value = valueConversionFunction(*argv);

        --argc;
        ++argv;

        return value;
    }

    std::string parseString(
        const std::string& argName,
        const std::string& currentArg,
        int& argc,
        const char**& argv) {

        return parseArgumentWithValue<std::string>(
            argName,
            currentArg,
            argc,
            argv,
            [](const char* arg) { return std::string(arg); }
        );
    }

    double parseSamplesPerSecond(const std::string& currentArg, int& argc, const char**& argv) {
        return parseArgumentWithValue<double>(
            samplesPerSecondOptionName,
            currentArg,
            argc,
            argv,
            [](const char* arg) {

                const auto samplesPerSecondString = std::string(arg);
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
        );
    }

    std::string parseTargetTraceFile(const std::string& currentArg, int& argc, const char**& argv) {
        return parseString(
            targetTraceFileOptionName,
            currentArg,
            argc,
            argv
        );
    }

    LogLevel parseLogLevel(const std::string& currentArg, int& argc, const char**& argv) {
        return parseArgumentWithValue<LogLevel>(
            logLevelOptionName,
            currentArg,
            argc,
            argv,
            [](const char* arg) {
                const auto logLevelString = std::string(arg);

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

        if (currentArg == samplesPerSecondOptionName) {
            options.samplesPerSecond = parseSamplesPerSecond(currentArg, argc, argv);
            continue;
        }

        if (currentArg == targetTraceFileOptionName) {
            options.targetTraceFile = parseTargetTraceFile(currentArg, argc, argv);
            continue;
        }

        if (currentArg == noTUIOptionName) {
            options.tui = false;
            --argc;
            ++argv;
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
        format_string(
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
