#include "swimps-profile.h"

#include <unistd.h>

swimps_error_code_t swimps_profile(const char* const executable) {
    const pid_t pid = fork();

    switch(pid) {
    // Fork failed.
    case -1:
        // TODO: log message.
        return SWIMPS_ERROR_FORK_FAILED;
    // We're the child process.
    case 0:
        // TODO: implement child process.
        return SWIMPS_ERROR_NONE;
    // We're the parent process.
    default:
        return SWIMPS_ERROR_NONE;
    }
}

