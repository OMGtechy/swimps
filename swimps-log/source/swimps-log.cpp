#include "swimps-log.h"

#include <string.h>
#include <unistd.h>

size_t swimps_format_log_message(
    const swimps::log::LogLevel logLevel,
    const char* const __restrict__ message,
    const size_t messageSize,
    char* __restrict__ targetBuffer,
    const size_t targetBufferSize) {

    assert(message != NULL);
    assert(targetBuffer != NULL);

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
        { targetBuffer, targetBufferSize }
    );

    bytesWritten += swimps::io::write_to_buffer(
        { message, messageSize },
        { targetBuffer + bytesWritten, targetBufferSize - bytesWritten }
    );

    const char newLine[] = { '\n' };
    bytesWritten += swimps::io::write_to_buffer(
        newLine,
        { targetBuffer + bytesWritten, targetBufferSize - bytesWritten }
    );

    return bytesWritten;
}

size_t swimps::log::write_to_log(
    const swimps::log::LogLevel logLevel,
    const char* const message,
    const size_t messageSize) {

    assert(message != NULL);

    char targetBuffer[2048] = { 0 };

    const size_t bytesWritten = swimps_format_log_message(logLevel,
                                                          message,
                                                          messageSize,
                                                          targetBuffer,
                                                          sizeof targetBuffer);

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

    return swimps::io::write_to_file_descriptor(targetBuffer,
                                           bytesWritten,
                                           targetFileDescriptor);
}
