#pragma once

#include "swimps-error.h"

#include <unistd.h>

namespace swimps::option {
    struct Options;
}

namespace swimps::profile {
    //!
    //! \brief  Starts a profile.
    //!
    //! \param[in]  options  The swimps options to use when profiling.
    //!
    //! \returns  An error code, if there was an error.
    //!
    swimps::error::ErrorCode start(const option::Options& options);

    //!
    //! \brief  Sets up a process in the "child" role for profiling.
    //!
    //! \param[in]  options  The swimps options to use when profiling.
    //!                                        followed by its args.
    //!
    //! \returns  An error code, if there was an error.
    //!
    //! \note  If successful, this function never returns.
    //!
    swimps::error::ErrorCode child(const option::Options& options);

    //!
    //! \brief  Sets up a process in the "parent" to monitor the profiled executable.
    //!
    //! \param[in]  The PID of the child process.
    //!
    //! \returns An error code, if there was an error.
    //!
    swimps::error::ErrorCode parent(const pid_t childPid);
}

