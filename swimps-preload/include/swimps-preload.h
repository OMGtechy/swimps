#pragma once

#include "swimps-trace.h"

namespace swimps::preload {
    //!
    //! \brief  Gets a backtrace from the currently running program.
    //!
    //! \returns The requested backtrace.
    //!
    swimps::trace::Backtrace get_backtrace();
}
