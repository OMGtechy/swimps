#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "swimps-error.h"

#include <unistd.h>

//!
//! \brief  Starts a profile.
//!
//! \param[in]  args  argv-esk parameters: the first should be the program to execute,
//!                                        followed by its args.
//!
//! \returns  An error code, if there was an error.
//!
swimps::error::ErrorCode swimps_profile(char** args);

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
swimps::error::ErrorCode swimps_profile_child(char** args);

//!
//! \brief  Sets up a process in the "parent" to monitor the profiled executable.
//!
//! \param[in]  The PID of the child process.
//!
//! \returns An error code, if there was an error.
//!
swimps::error::ErrorCode swimps_profile_parent(const pid_t childPid);

#ifdef __cplusplus
}
#endif

