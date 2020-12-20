#pragma once

#include <cstdio>
#include <cstdlib>

#include "signal.h"

#define swimps_assert(assertion) \
do { \
    if (!(assertion)) { \
        /* Block out all signals to we can use non-async-signal-safe functions. */ \
        sigset_t signalsToBlock; \
        sigset_t oldSignalsToBlock; \
        sigfillset(&signalsToBlock); \
        sigprocmask(SIG_BLOCK, &signalsToBlock, &oldSignalsToBlock); \
\
        fprintf( \
            stderr, \
            "Assertion %s failed at " __FILE__ ":%d\n", \
            (#assertion), \
            __LINE__ \
        ); \
\
        /* Reenable old signals so that SIGABRT can trigger correctly. */ \
        sigprocmask(SIG_SETMASK, &oldSignalsToBlock, nullptr); \
\
        abort(); \
    } \
} while(0)
