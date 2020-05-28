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

        snprintf(logMessageBuffer,
                 sizeof logMessageBuffer,
                 "fork failed, errno %d.",
                 errno);

        char formattedLogMessageBuffer[128] = { 0 };
        const size_t formattedLogBytesWritten = swimps_format_log_message(SWIMPS_LOG_LEVEL_DEBUG,
                                                                          logMessageBuffer,
                                                                          strlen(logMessageBuffer),
                                                                          formattedLogMessageBuffer,
                                                                          sizeof formattedLogMessageBuffer);

        swimps_write_to_file_descriptor(formattedLogMessageBuffer,
                                        formattedLogBytesWritten,
                                        STDERR_FILENO);

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

