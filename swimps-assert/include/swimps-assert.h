#pragma once

#include "swimps-log.h"
#include "swimps-io.h"

#include <stdlib.h>

#define swimps_assert(assertion) \
do { \
    if (!(assertion)) { \
        const char formatString[] = "Assertion %s failed at " __FILE__ ":%d"; \
\
        swimps::log::format_and_write_to_log<2048>( \
            swimps::log::LogLevel::Fatal, \
            formatString, \
            sizeof formatString, \
            (#assertion), \
            __LINE__ \
        ); \
\
        abort(); \
    } \
} while(0)
