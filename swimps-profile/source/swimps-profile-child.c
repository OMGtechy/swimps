#include "swimps-profile.h"
#include "swimps-log.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>

extern char** environ;

swimps_error_code_t swimps_profile_child(char** args) {
    if (args == NULL) {
        char message[] = "swimps_profile_child given NULL args";
        swimps_write_to_log(SWIMPS_LOG_LEVEL_FATAL,
                            message,
                            sizeof message);

        return SWIMPS_ERROR_NULL_PARAMETER;
    }

    if (args[0] == NULL) {
        char message[] = "No program specified.";
        swimps_write_to_log(SWIMPS_LOG_LEVEL_FATAL,
                            message,
                            sizeof message);

        return SWIMPS_ERROR_INVALID_PARAMETER;
    }

    // Enable tracing of the program we're about to exec into
    if (ptrace(PTRACE_TRACEME) == -1) {
        char logMessageBuffer[128] = { 0 };
        const size_t bytesWritten = snprintf(logMessageBuffer,
                                             sizeof logMessageBuffer,
                                             "ptrace(PTRACE_TRACEME) failed, errno %d (%s).",
                                             errno,
                                             strerror(errno));

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
    const char* const ldPreloadStr = "LD_PRELOAD=";
    const size_t ldPreloadStrLen = strlen(ldPreloadStr);
    char absolutePathToLDPreload[PATH_MAX] = { 0 };
    strncat(absolutePathToLDPreload, ldPreloadStr, ldPreloadStrLen);

    const ssize_t swimpsPathBufferBytes = readlink(
        "/proc/self/exe",
        absolutePathToLDPreload + ldPreloadStrLen,
        (sizeof absolutePathToLDPreload) - ldPreloadStrLen
    );

    if (swimpsPathBufferBytes == 0) {
        char logMessageBuffer[128] = { 0 };
        const size_t bytesWritten = snprintf(logMessageBuffer,
                                             sizeof logMessageBuffer,
                                             "readlink failed, errno %d.",
                                             errno);

        swimps_write_to_log(SWIMPS_LOG_LEVEL_FATAL,
                            logMessageBuffer,
                            bytesWritten);

        return SWIMPS_ERROR_READLINK_FAILED;
    }

    const char* preloadLibPathSuffix = "-preload/libswimps-preload.so";
    strncat(absolutePathToLDPreload, preloadLibPathSuffix, strlen(preloadLibPathSuffix));

    environment[i++] = absolutePathToLDPreload;
    environment[i] = NULL;

    {
        char logMessageBuffer[1024] = { 0 };
        char* logMessageBufferPtr = logMessageBuffer;
        char* const logMessageBufferPtrEnd = logMessageBuffer + (sizeof logMessageBuffer);
        size_t totalBytesWritten = snprintf(logMessageBufferPtr,
                                            logMessageBufferPtrEnd - logMessageBufferPtr,
                                            "Executing program:");

        logMessageBufferPtr += totalBytesWritten;

        for(char** argToPrint = args; *argToPrint != NULL && logMessageBufferPtr < logMessageBufferPtrEnd; ++argToPrint) {
            const size_t bytesWritten = snprintf(logMessageBufferPtr,
                                                 logMessageBufferPtrEnd - logMessageBufferPtr,
                                                 " %s",
                                                 *argToPrint);

            logMessageBufferPtr += bytesWritten;
            totalBytesWritten += bytesWritten;
        }

        swimps_write_to_log(SWIMPS_LOG_LEVEL_INFO,
                            logMessageBuffer,
                            totalBytesWritten);
    }

    for(char** envIter = environment; *envIter != NULL; ++envIter) {
        swimps_write_to_log(SWIMPS_LOG_LEVEL_DEBUG,
                            *envIter,
                            strlen(*envIter));
    }

    execve(*args, args, environment);

    // we only get here if execve failed
    {
        char logMessageBuffer[128] = { 0 };
        const size_t bytesWritten = snprintf(logMessageBuffer,
                                             sizeof logMessageBuffer,
                                             "Failed to execute target program, errno %d (%s).",
                                             errno,
                                             strerror(errno));

        swimps_write_to_log(SWIMPS_LOG_LEVEL_FATAL,
                            logMessageBuffer,
                            bytesWritten);
    }

    return SWIMPS_ERROR_EXECVE_FAILED;
}
