#include "swimps-log/swimps-log.h"

#include <cstring>

#include <unistd.h>

#include <signalsafe/file.hpp>

using signalsafe::File;

namespace {
    static swimps::log::LogLevel logLevelFilter = swimps::log::LogLevel::Debug;
}

void swimps::log::setLevelToLog(LogLevel logLevel) noexcept {
    logLevelFilter = logLevel;
}

size_t swimps::log::format_message(
    const swimps::log::LogLevel logLevel,
    swimps::container::Span<const char> message,
    swimps::container::Span<char> target) {

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

    newBytesWritten = swimps::io::write_to_buffer(
        { *logLevelString, sizeof(*logLevelString) - 1 },
        target
    );

    totalBytesWritten += newBytesWritten;
    target += newBytesWritten;

    newBytesWritten = swimps::io::write_to_buffer(
        message,
        target
    );

    totalBytesWritten += newBytesWritten;
    target += newBytesWritten;

    const char newLine[] = { '\n' };
    newBytesWritten = swimps::io::write_to_buffer(
        newLine,
        target
    );

    totalBytesWritten += newBytesWritten;
    target += newBytesWritten;

    return totalBytesWritten;
}

size_t swimps::log::write_to_log(
    const swimps::log::LogLevel logLevel,
    swimps::container::Span<const char> message) {

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
