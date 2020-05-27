#include "swimps-log.h"
#include "swimps-io.h"

#include "string.h"

size_t swimps_format_log_message(
    const swimps_log_level_t logLevel,
    const char* const __restrict__ message,
    const size_t messageSize,
    char* __restrict__ targetBuffer,
    const size_t targetBufferSize) {

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
