#pragma once

#include "swimps-trace.h"

#include <tuple>

#include <ucontext.h>

namespace swimps::preload {
    using get_backtrace_result_t =
            std::tuple<swimps::trace::Backtrace,
                       swimps::trace::StackFrame[64]>;

    //!
    //! \brief  Gets a backtrace from the currently running program.
    //!         This is designed to be used from a signal handler.
    //!
    //! \param[in]      context           The signal handler context.
    //! \param[in,out]  nextBacktraceID   The counter for the next backtrace ID.
    //! \param[in,out]  nextStackFrameID  The counter for the next stack frame ID.
    //!
    //! \returns The requested backtrace and the stack frames associated with it.
    //!
    //! \note  Modifies the backtrace and stack frame ids.
    //!
    get_backtrace_result_t get_backtrace(
        ucontext_t* context,
        swimps::trace::backtrace_id_t& nextBacktraceID,
        swimps::trace::stack_frame_id_t& nextStackFrameID);
}
