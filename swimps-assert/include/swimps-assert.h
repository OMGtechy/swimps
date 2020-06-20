#pragma once

#include "swimps-log.h"
#include "swimps-io.h"

#include <stdlib.h>

#define swimps_assert(assertion) \
do { \
    if (!(assertion)) { \
        char message[2048] = { 0 }; \
        const char formatString[] = "Assertion %s failed at " __FILE__ ":%d"; \
\
        const size_t bytesWritten = swimps_format_string( \
            formatString, \
            sizeof formatString, \
            message, \
            sizeof message, \
            (#assertion), \
            __LINE__ \
        ); \
\
        swimps_write_to_log( \
            SWIMPS_LOG_LEVEL_FATAL, \
            message, \
            bytesWritten \
        ); \
\
        abort(); \
    } \
} while(0)
