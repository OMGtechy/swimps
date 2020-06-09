#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "swimps-error.h"

#include <unistd.h>

//!
//! \brief  Starts a profile.
//!
//! \param[in]  executable  The executable to profile.
//!
//! \returns  An error code, if there was an error.
//!
swimps_error_code_t swimps_profile(const char* const executable);

//!
//! \brief  Sets up a process in the "child" role for profiling.
//!
//! \param[in]  executable  The executable to profile.
//!
//! \returns  An error code, if there was an error.
//!
//! \note  If successful, this function never returns.
//!
swimps_error_code_t swimps_profile_child(const char* const executable);

//!
//! \brief  Sets up a process in the "parent" to monitor the profiled executable.
//!
//! \param[in]  The PID of the child process.
//!
//! \returns An error code, if there was an error.
//!
swimps_error_code_t swimps_profile_parent(const pid_t childPid);

#ifdef __cplusplus
}
#endif

