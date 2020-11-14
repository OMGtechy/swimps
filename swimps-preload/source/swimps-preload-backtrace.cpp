#include "swimps-preload.h"

#define UNW_LOCAL_ONLY
#include <libunwind.h>

swimps::trace::Backtrace swimps::preload::get_backtrace() {
    unw_context_t unwindContext;
    unw_getcontext(&unwindContext);

    unw_cursor_t unwindCursor;
    unw_init_local(&unwindCursor, &unwindContext);

    swimps::trace::Backtrace result;

    bool thereIsAnotherStackFrame = true;
    for(size_t stackFrameIndex = 0;
        thereIsAnotherStackFrame && stackFrameIndex < sizeof result.stackFrames;
        ++stackFrameIndex) {

        auto& stackFrame = result.stackFrames[stackFrameIndex];

        unw_word_t offset = 0;

        const auto getProcNameResult = unw_get_proc_name(
            &unwindCursor,
            &stackFrame.mangledFunctionName[0],
            sizeof stackFrame.mangledFunctionName - 1,
            &offset
        );

        if (getProcNameResult != 0) {
            stackFrame.mangledFunctionName[0] = '\0';
        }

        result.stackFrameCount += 1;
        stackFrame.offset = getProcNameResult == 0 ? static_cast<int64_t>(offset) : 0;
        stackFrame.mangledFunctionNameLength = getProcNameResult == 0 ? strlen(stackFrame.mangledFunctionName) : 0;

        thereIsAnotherStackFrame = unw_step(&unwindCursor) == 1;
    }

    return result;
}

