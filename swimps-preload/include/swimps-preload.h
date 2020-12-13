#pragma once

#include "swimps-trace.h"

#include <ucontext.h>

namespace swimps::preload {
    //!
    //! \brief  Gets a backtrace from the currently running program.
    //!         This is designed to be used from
    //!
    //! \param[in]      context           The signal handler context.
    //! \param[in,out]  nextBacktraceID   The counter for the next backtrace ID.
    //! \param[in,out]  nextStackFrameID  The counter for the next stack frame ID.
    //!
    //! \returns The requested backtrace.
    //!
    //! \note  Modifies the backtrace and stack frame ids.
    //!
    swimps::trace::Backtrace get_backtrace(
        ucontext_t* context,
        swimps::trace::backtrace_id_t& nextBacktraceID,
        swimps::trace::stack_frame_id_t& nextStackFrameID);
}
