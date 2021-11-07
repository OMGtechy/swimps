#include "swimps-log/swimps-log.h"

#include <cstring>

#include <unistd.h>

#include <signalsafe/file.hpp>
#include <signalsafe/memory.hpp>

using signalsafe::File;
using signalsafe::memory::copy_no_overlap;

namespace {
    static swimps::log::LogLevel logLevelFilter = swimps::log::LogLevel::Debug;
}

void swimps::log::setLevelToLog(LogLevel logLevel) noexcept {
    logLevelFilter = logLevel;
}

size_t swimps::log::format_message(
    const swimps::log::LogLevel logLevel,
    std::span<const char> message,
    std::span<char> target) {

    constexpr size_t logLevelStringSize = 15;
    const char (*logLevelString)[logLevelStringSize] = nullptr;

    switch(logLevel) {
    case swimps::log::LogLevel::Fatal:   logLevelString = &"SWIMPS: FTL - "; break;
    case swimps::log::LogLevel::Error:   logLevelString = &"SWIMPS: ERR - "; break;
    case swimps::log::LogLevel::Warning: logLevelString = &"SWIMPS: WRN - "; break;
    case swimps::log::LogLevel::Info:    logLevelString = &"SWIMPS: INF - "; break;
    case swimps::log::LogLevel::Debug:   logLevelString = &"SWIMPS: DBG - "; break;
    default:                             logLevelString = &"SWIMPS: ??? - "; break;
    }

    swimps_assert(logLevelString != nullptr);

    size_t totalBytesWritten = 0;
    size_t newBytesWritten = 0;

    newBytesWritten = copy_no_overlap(
        std::span<const char>{ *logLevelString, sizeof(*logLevelString) - 1 },
        target
    );

    totalBytesWritten += newBytesWritten;
    target = target.last(target.size() - newBytesWritten);

    newBytesWritten = copy_no_overlap(
        message,
        target
    );

    totalBytesWritten += newBytesWritten;
    target = target.last(target.size() - newBytesWritten);

    const char newLine[] = { '\n' };
    newBytesWritten = copy_no_overlap(
        newLine,
        target
    );

    totalBytesWritten += newBytesWritten;
    target = target.last(target.size() - newBytesWritten);

    return totalBytesWritten;
}

size_t swimps::log::write_to_log(
    const swimps::log::LogLevel logLevel,
    std::span<const char> message) {

    //! Log levels are in decenting order of severity
    if (static_cast<int8_t>(logLevel) > static_cast<int8_t>(logLevelFilter)) {
        return 0;
    }

    char targetBuffer[2048] = { 0 };

    const size_t bytesWritten = swimps::log::format_message(
        logLevel,
        message,
        targetBuffer
    );

    File& targetFile = [](const swimps::log::LogLevel ll) -> File& {
        switch(ll) {
        case swimps::log::LogLevel::Fatal:
        case swimps::log::LogLevel::Error:
        case swimps::log::LogLevel::Warning:
            return signalsafe::standard_error();
        default:
            return signalsafe::standard_output();
        }
    }(logLevel);

    return targetFile.write({ targetBuffer, bytesWritten });
}
