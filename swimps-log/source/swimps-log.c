#include "swimps-log.h"
#include "swimps-io.h"

#include <string.h>
#include <unistd.h>
#include <assert.h>

size_t swimps_format_log_message(
    const swimps_log_level_t logLevel,
    const char* const __restrict__ message,
    const size_t messageSize,
    char* __restrict__ targetBuffer,
    const size_t targetBufferSize) {

    assert(message != NULL);
    assert(targetBuffer != NULL);

    const char* logLevelString = NULL;

    switch(logLevel) {
    case SWIMPS_LOG_LEVEL_FATAL:   logLevelString = "SWIMPS: FTL - "; break;
    case SWIMPS_LOG_LEVEL_ERROR:   logLevelString = "SWIMPS: ERR - "; break;
    case SWIMPS_LOG_LEVEL_WARNING: logLevelString = "SWIMPS: WRN - "; break;
    case SWIMPS_LOG_LEVEL_INFO:    logLevelString = "SWIMPS: INF - "; break;
    case SWIMPS_LOG_LEVEL_DEBUG:   logLevelString = "SWIMPS: DBG - "; break;
    default:                       logLevelString = "SWIMPS: ??? - "; break;
    }

    size_t bytesWritten = swimps_write_to_buffer(logLevelString,
                                                 strlen(logLevelString),
                                                 targetBuffer,
                                                 targetBufferSize);

    bytesWritten += swimps_write_to_buffer(message,
                                           messageSize,
                                           targetBuffer + bytesWritten,
                                           targetBufferSize - bytesWritten);

    const char newLine[] = { '\n' };
    bytesWritten += swimps_write_to_buffer(newLine,
                                           sizeof newLine,
                                           targetBuffer + bytesWritten,
                                           targetBufferSize - bytesWritten);

    return bytesWritten;
}

size_t swimps_write_to_log(
    const swimps_log_level_t logLevel,
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
    case SWIMPS_LOG_LEVEL_FATAL:
    case SWIMPS_LOG_LEVEL_ERROR:
    case SWIMPS_LOG_LEVEL_WARNING:
        targetFileDescriptor = STDERR_FILENO;
        break;
    default:
        targetFileDescriptor = STDOUT_FILENO;
        break;
    }

    return swimps_write_to_file_descriptor(targetBuffer,
                                           bytesWritten,
                                           targetFileDescriptor);
}

size_t swimps_format_and_write_to_log(
    const swimps_log_level_t logLevel,
    const char* const __restrict__ formatBuffer,
    const size_t formatBufferSize,
    char* __restrict__ targetBuffer,
    const size_t targetBufferSize,
    ...) {

    assert(formatBuffer != NULL);
    assert(targetBuffer != NULL);

    va_list varargs;
    va_start(varargs, targetBufferSize);

    const size_t bytesWritten = swimps_format_string_valist(
        formatBuffer,
        formatBufferSize,
        targetBuffer,
        targetBufferSize,
        varargs
    );

    va_end(varargs);

    return swimps_write_to_log(
        logLevel,
        targetBuffer,
        bytesWritten
    );
}
