#pragma once

#include <cstdio>
#include <cstdlib>

#include <signal.h>
#include <unistd.h>

#define swimps_assert(assertion) \
do { \
    if (!(assertion)) { \
        /* Block out all signals so it's saf*er* to use non-async-signal-safe functions. \
           It's still not safe, but we're about to abort anyway so let's just go for it. */ \
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
        /* note that you'll probably want to disable ptrace if you're going to use this */ \
        if (std::getenv("SWIMPS_ASSERT_WAIT_FOR_DEBUGGER") != nullptr) { \
            fprintf(stderr, "Process %d: waiting for debugger.\n", getpid()); \
            /* you can set this to false in your debugger if you want to continue */ \
            volatile bool waitForDebugger = true; \
            do { sleep(1); } while(waitForDebugger); \
        } \
\
        /* Reenable old signals so that SIGABRT can trigger correctly. */ \
        sigprocmask(SIG_SETMASK, &oldSignalsToBlock, nullptr); \
\
        abort(); \
    } \
} while(0)
