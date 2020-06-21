#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum swimps_error_code {
    SWIMPS_ERROR_NONE,
    SWIMPS_ERROR_NULL_PARAMETER,
    SWIMPS_ERROR_INVALID_PARAMETER,
    SWIMPS_ERROR_FORK_FAILED,
    SWIMPS_ERROR_PTRACE_FAILED,
    SWIMPS_ERROR_READLINK_FAILED,
    SWIMPS_ERROR_EXECVE_FAILED
} swimps_error_code_t;

#ifdef __cplusplus
}
#endif

