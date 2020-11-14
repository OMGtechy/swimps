#pragma once

#include "swimps-log.h"
#include "swimps-io.h"

#include <stdlib.h>

#define swimps_assert(assertion) \
do { \
    if (!(assertion)) { \
        swimps::log::format_and_write_to_log<2048>( \
            swimps::log::LogLevel::Fatal, \
            "Assertion %s failed at " __FILE__ ":%d", \
            (#assertion), \
            __LINE__ \
        ); \
\
        abort(); \
    } \
} while(0)
