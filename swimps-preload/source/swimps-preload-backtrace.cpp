#include "swimps-preload.h"

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#define S(x) S2(x)
#define S2(x) #x

#pragma message "libunwind version: " S(UNW_VERSION_MAJOR) "." S(UNW_VERSION_MINOR) "." S(UNW_VERSION_EXTRA)

using swimps::trace::address_t;

swimps::preload::get_backtrace_result_t swimps::preload::get_backtrace(ucontext_t* context, swimps::trace::backtrace_id_t& nextBacktraceID, swimps::trace::stack_frame_id_t& nextStackFrameID) {
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

    get_backtrace_result_t result;
    auto& backtrace = std::get<0>(result);
    auto& stackFrames = std::get<1>(result);
    backtrace.id = nextBacktraceID++;

    bool thereIsAnotherStackFrame = true;
    for(size_t stackFrameIndex = 0;
        thereIsAnotherStackFrame && stackFrameIndex < std::size(stackFrames);
        ++stackFrameIndex) {

        auto& stackFrame = stackFrames[stackFrameIndex];
        stackFrame.id = nextStackFrameID++;
        backtrace.stackFrameIDs[stackFrameIndex] = stackFrame.id;
        backtrace.stackFrameIDCount += 1;

        unw_word_t instructionPointer = 0;
        unw_get_reg(&unwindCursor, UNW_REG_IP, &instructionPointer);
        stackFrame.instructionPointer = static_cast<address_t>(instructionPointer);

        thereIsAnotherStackFrame = unw_step(&unwindCursor) == 1;
    }

    return result;
}

