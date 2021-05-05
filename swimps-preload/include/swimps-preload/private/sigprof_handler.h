#pragma once

#include <signal.h>

namespace swimps::preload {
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