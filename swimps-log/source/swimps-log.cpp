#include "swimps-log.h"

#include <string.h>
#include <unistd.h>

size_t swimps_format_log_message(
    const swimps::log::LogLevel logLevel,
    swimps::container::Span<const char> message,
    swimps::container::Span<char> target) {

    const char* logLevelString = NULL;

    switch(logLevel) {
    case swimps::log::LogLevel::Fatal:   logLevelString = "SWIMPS: FTL - "; break;
    case swimps::log::LogLevel::Error:   logLevelString = "SWIMPS: ERR - "; break;
    case swimps::log::LogLevel::Warning: logLevelString = "SWIMPS: WRN - "; break;
    case swimps::log::LogLevel::Info:    logLevelString = "SWIMPS: INF - "; break;
    case swimps::log::LogLevel::Debug:   logLevelString = "SWIMPS: DBG - "; break;
    default:                             logLevelString = "SWIMPS: ??? - "; break;
    }

    size_t bytesWritten = swimps::io::write_to_buffer(
        { logLevelString, strlen(logLevelString) },
        target
    );

    target += bytesWritten;

    bytesWritten += swimps::io::write_to_buffer(
        message,
        target
    );

    target += bytesWritten;

    const char newLine[] = { '\n' };
    bytesWritten += swimps::io::write_to_buffer(
        newLine,
        target
    );

    return bytesWritten;
}

size_t swimps::log::write_to_log(
    const swimps::log::LogLevel logLevel,
    swimps::container::Span<const char> message) {

    char targetBuffer[2048] = { 0 };

    const size_t bytesWritten = swimps_format_log_message(
        logLevel,
        message,
        targetBuffer
    );

    int targetFileDescriptor;

    switch(logLevel) {
    case swimps::log::LogLevel::Fatal:
    case swimps::log::LogLevel::Error:
    case swimps::log::LogLevel::Warning:
        targetFileDescriptor = STDERR_FILENO;
        break;
    default:
        targetFileDescriptor = STDOUT_FILENO;
        break;
    }

    return swimps::io::write_to_file_descriptor(
        { targetBuffer, bytesWritten },
        targetFileDescriptor
    );
}
