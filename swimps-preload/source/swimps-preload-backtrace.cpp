#include "swimps-preload.h"

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#define S(x) S2(x)
#define S2(x) #x

#pragma message "libunwind version: " S(UNW_VERSION_MAJOR) "." S(UNW_VERSION_MINOR) "." S(UNW_VERSION_EXTRA)

swimps::trace::Backtrace swimps::preload::get_backtrace(ucontext_t* context, swimps::trace::backtrace_id_t& nextBacktraceID, swimps::trace::stack_frame_id_t& nextStackFrameID) {
    unw_context_t unwindContext;

    #ifdef __aarch64__
        constexpr int flags = UNW_INIT_SIGNAL_FRAME;
        memcpy(&unwindContext, context, sizeof unwindContext);
    #else
        (void)context;
        constexpr int flags = 0;
        unw_getcontext(&unwindContext);
    #endif

    unw_cursor_t unwindCursor;

    #if UNW_VERSION < UNW_VERSION_CODE(1,3)
        #ifdef __aarch64__
            #error "AArch64 requires libunwind 1.3 or greater."
        #endif
        (void)flags;
        unw_init_local(&unwindCursor, &unwindContext);
    #else
        unw_init_local2(&unwindCursor, &unwindContext, flags);
    #endif

    swimps::trace::Backtrace result;
    result.id = nextBacktraceID++;

    bool thereIsAnotherStackFrame = true;
    for(size_t stackFrameIndex = 0;
        thereIsAnotherStackFrame && stackFrameIndex < sizeof result.stackFrames;
        ++stackFrameIndex) {

        auto& stackFrame = result.stackFrames[stackFrameIndex];
        stackFrame.id = nextStackFrameID++;

        unw_word_t offset = 0;

        constexpr auto maxMangledFunctionNameLength = sizeof stackFrame.mangledFunctionName - 1;

        const auto getProcNameResult = unw_get_proc_name(
            &unwindCursor,
            &stackFrame.mangledFunctionName[0],
            maxMangledFunctionNameLength,
            &offset
        );

        if (getProcNameResult != 0) {
            stackFrame.mangledFunctionName[0] = '\0';
        }

        result.stackFrameCount += 1;
        stackFrame.offset = getProcNameResult == 0 ? static_cast<int64_t>(offset) : 0;
        stackFrame.mangledFunctionNameLength = getProcNameResult == 0 ? strnlen(stackFrame.mangledFunctionName, maxMangledFunctionNameLength) : 0;

        thereIsAnotherStackFrame = unw_step(&unwindCursor) == 1;
    }

    return result;
}

