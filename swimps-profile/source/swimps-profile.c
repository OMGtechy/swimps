#include "swimps-profile.h"
#include "swimps-log.h"
#include "swimps-io.h"

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <stdlib.h>
#include <linux/limits.h>

extern char** environ;

swimps_error_code_t child(const char* const executable) {
    // Enable tracing of the program we're about to exec into
    if (ptrace(PTRACE_TRACEME) == -1) {
        char logMessageBuffer[128] = { 0 };
        const size_t bytesWritten = snprintf(logMessageBuffer,
                                             sizeof logMessageBuffer,
                                             "ptrace(PTRACE_TRACEME) failed, errno %d.",
                                             errno);

        swimps_write_to_log(SWIMPS_LOG_LEVEL_FATAL,
                            logMessageBuffer,
                            bytesWritten);

        return SWIMPS_ERROR_PTRACE_FAILED;
    }

    // Work out how big the environment is
    size_t environmentSize = 0;
    while(environ[environmentSize] != NULL) {
        environmentSize += 1;
    }

    // Add one for LD_PRELOAD and another for the NULL at the end
    environmentSize += 2;

    // Allocate space for the environment
    char** environment = malloc(environmentSize * sizeof(char*));

    // Copy over the existing environment
    size_t i = 0;
    for(; environ[i] != NULL; ++i) {
        environment[i] = environ[i];
    }

    // Add LD_PRELOAD
    // In its current form this requires the library to be in the same working directory
    // This will have to be changed in future.
    const char* const ldPreloadStr = "LD_PRELOAD=";
    const size_t ldPreloadStrLen = strlen(ldPreloadStr);
    char absolutePathToLDPreload[PATH_MAX] = { 0 };
    strncat(absolutePathToLDPreload, ldPreloadStr, ldPreloadStrLen);

    const char* getCwdResult = getcwd(
        absolutePathToLDPreload + ldPreloadStrLen,
        (sizeof absolutePathToLDPreload) - ldPreloadStrLen
    );

    if (getCwdResult == NULL) {
        char logMessageBuffer[128] = { 0 };
        const size_t bytesWritten = snprintf(logMessageBuffer,
                                             sizeof logMessageBuffer,
                                             "getcwd failed, errno %d.",
                                             errno);

        swimps_write_to_log(SWIMPS_LOG_LEVEL_FATAL,
                            logMessageBuffer,
                            bytesWritten);

        return SWIMPS_ERROR_GETCWD_FAILED;
    }

    const char* preloadLibFileName = "/libswimps-preload.so";
    strncat(absolutePathToLDPreload, preloadLibFileName, strlen(preloadLibFileName));

    environment[i++] = absolutePathToLDPreload;
    environment[i] = NULL;

    // TODO: support user passing arguments to program
    char* argv[] = {
        strdup(executable),
        NULL
    };

    // TODO: include args
    {
        char logMessageBuffer[1024] = { 0 };
        const size_t bytesWritten = snprintf(logMessageBuffer,
                                             sizeof logMessageBuffer,
                                             "Executing program: %s",
                                             executable);

        swimps_write_to_log(SWIMPS_LOG_LEVEL_INFO,
                            logMessageBuffer,
                            bytesWritten);
    }

    for(char** envIter = environment; *envIter != NULL; ++envIter) {
        swimps_write_to_log(SWIMPS_LOG_LEVEL_DEBUG,
                            *envIter,
                            strlen(*envIter));
    }

    execve(executable, argv, environment);

    // we only get here if execve failed
    {
        char logMessageBuffer[128] = { 0 };
        const size_t bytesWritten = snprintf(logMessageBuffer,
                                             sizeof logMessageBuffer,
                                             "Failed to execute target program, errno %d.",
                                             errno);

        swimps_write_to_log(SWIMPS_LOG_LEVEL_FATAL,
                            logMessageBuffer,
                            bytesWritten);
    }

    for(size_t i = 0; i < ((sizeof argv)  / (sizeof argv[0])); ++i) {
        free(argv[i]);
    }

    return SWIMPS_ERROR_EXECVE_FAILED;
}

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
        return child(executable);
    default:
        // We're the parent process.
        // TODO: implement parent
        return SWIMPS_ERROR_NONE;
    }
}

