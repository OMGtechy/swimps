#include "swimps-profile.h"
#include "swimps-log.h"
#include "swimps-io.h"

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

swimps_error_code_t swimps_profile(char** args) {
    const pid_t pid = fork();

    switch(pid) {
    case -1: {
        // Fork failed.
        char logMessageBuffer[128] = { 0 };

        const size_t bytesWritten = snprintf(logMessageBuffer,
                                             sizeof logMessageBuffer,
                                             "fork failed, errno %d (%s).",
                                             errno,
                                             strerror(errno));

        swimps::log::write_to_log(swimps::log::LogLevel::Fatal,
                            logMessageBuffer,
                            bytesWritten);

        return SWIMPS_ERROR_FORK_FAILED;
    }
    case 0:
        return swimps_profile_child(args);
    default:
        return swimps_profile_parent(pid);
    }
}

