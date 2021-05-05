#include "swimps-option/swimps-option-options.h"

#include "swimps-assert/swimps-assert.h"

#include <sstream>

namespace {
    const std::string stringOptionsHelpLabel = "help ";
    const std::string stringOptionsTUILabel = "tui ";
    const std::string stringOptionsLogLevelLabel = "log-level ";
    const std::string stringOptionsSamplesPerSecondLabel = "samples-per-second ";
    const std::string stringOptionsTargetTraceFileLabel = "target-trace-file ";
    const std::string stringOptionsTargetProgramLabel = "target-program ";
    const std::string stringOptionsTargetProgramArgsLabel = "target-program-args ";
    const std::string stringOptionsLoadLabel = "load ";

    std::string chompPrefix(std::string string, std::string prefix) {
        swimps_assert(string.starts_with(prefix));
        return string.substr(prefix.size());
    }
}

using swimps::log::LogLevel;
using swimps::option::Options;

Options swimps::option::Options::fromString(std::string string) {
    Options result;

    // help
    string = chompPrefix(string, stringOptionsHelpLabel);
    swimps_assert(string.length() >= 1);
    result.help = string[0] == '1';
    string = chompPrefix(string.substr(1), "|");

    // tui
    string = chompPrefix(string, stringOptionsTUILabel);
    swimps_assert(string.length() >= 1);
    result.tui = string [0] == '1';
    string = chompPrefix(string.substr(1), "|");

    // load
    string = chompPrefix(string, stringOptionsLoadLabel);
    swimps_assert(string.length() >= 1);
    result.load = string[0] == '1';
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

    // tui
    stringStream << stringOptionsTUILabel << (tui ? "1" : "0") << "|";

    // load
    stringStream << stringOptionsLoadLabel << (load ? "1" : "0") << "|";

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

