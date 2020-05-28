#include "swimps-profile.h"
#include "swimps-log.h"
#include "swimps-io.h"

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

swimps_error_code_t swimps_profile(const char* const executable) {
    const pid_t pid = fork();

    switch(pid) {
    case -1: {
        // Fork failed.
        char logMessageBuffer[128] = { 0 };

        const size_t bytesWritten = snprintf(logMessageBuffer,
                                             sizeof logMessageBuffer,
                                             "fork failed, errno %d.",
                                             errno);

        swimps_write_to_log(SWIMPS_LOG_LEVEL_FATAL,
                            logMessageBuffer,
                            bytesWritten);

        return SWIMPS_ERROR_FORK_FAILED;
    }
    case 0:
        // We're the child process.
        // TODO: implement child process.
        (void) executable;
        return SWIMPS_ERROR_NONE;
    default:
        // We're the parent process.
        return SWIMPS_ERROR_NONE;
    }
}

