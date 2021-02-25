#pragma once

#include "swimps-trace.h"

#include <tuple>
#include <array>

#include <ucontext.h>

namespace swimps::preload {
    using get_backtrace_result_t =
            std::tuple<swimps::trace::Backtrace,
                       std::array<swimps::trace::RawStackFrame, 64>>;

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

    //!
    //! \brief  Gets the proc map for the current process.
    //!
    //! \returns  The proc map for the current process.
    //!
    //! \note  This is *not* async signal safe.
    //!
    swimps::trace::ProcMaps get_proc_maps();

    //!
    //! \brief  The sampling signal handler.
    //!
    //! \param[in]  unused   Unused.
    //! \param[in]  unused   Unused.
    //! \param[in]  context  Signal context information.
    //!
    //! \note  This function is async signal safe.
    //!
    void sigprof_handler(const int, siginfo_t*, void* context);
}
