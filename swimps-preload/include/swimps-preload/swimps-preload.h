#pragma once

#include "swimps-trace/swimps-trace.h"

#include <tuple>
#include <array>

#include <ucontext.h>

namespace swimps::preload {
    //!
    //! \brief  Gets the proc map for the current process.
    //!
    //! \returns  The proc map for the current process.
    //!
    //! \note  This is *not* async signal safe.
    //!
    swimps::trace::ProcMaps get_proc_maps();
}
