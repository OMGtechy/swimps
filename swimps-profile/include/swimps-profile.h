#pragma once

#include "swimps-error.h"

#include <unistd.h>

namespace swimps::profile {
    //!
    //! \brief  Starts a profile.
    //!
    //! \param[in]  args  argv-esk parameters: the first should be the program to execute,
    //!                                        followed by its args.
    //!
    //! \returns  An error code, if there was an error.
    //!
    swimps::error::ErrorCode start(char** args);

    //!
    //! \brief  Sets up a process in the "child" role for profiling.
    //!
    //! \param[in]  args  argv-esk parameters: the first should be the program to execute,
    //!                                        followed by its args.
    //!
    //! \returns  An error code, if there was an error.
    //!
    //! \note  If successful, this function never returns.
    //!
    swimps::error::ErrorCode child(char** args);

    //!
    //! \brief  Sets up a process in the "parent" to monitor the profiled executable.
    //!
    //! \param[in]  The PID of the child process.
    //!
    //! \returns An error code, if there was an error.
    //!
    swimps::error::ErrorCode parent(const pid_t childPid);
}

