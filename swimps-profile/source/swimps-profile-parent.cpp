#include "swimps-profile.h"
#include "swimps-io.h"
#include "swimps-log.h"

#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <sys/ptrace.h>

swimps_error_code_t swimps_profile_parent(const pid_t childPid) {
    while(1) {
        int status = 0;
        waitpid(childPid, &status, 0);

        if (WIFEXITED(status)) {
            const char message[] = "Child process exited normally.";
            swimps::log::write_to_log(swimps::log::LogLevel::Debug,
                                message,
                                strlen(message));
            return SWIMPS_ERROR_NONE;
        }

        if (WIFSIGNALED(status)) {
            const char message[] = "Child process exited due to a signal.";
            swimps::log::write_to_log(swimps::log::LogLevel::Debug,
                                message,
                                strlen(message));
            return SWIMPS_ERROR_NONE;
        }

        if (WIFSTOPPED(status)) {
            const int signalNumber = WSTOPSIG(status);

            {
                char targetBuffer[128] = { 0 };
                const char formatBuffer[] = "Child process stopped due to signal %d (%s).";
                const size_t bytesWritten = swimps_format_string(formatBuffer,
                                                                 strlen(formatBuffer),
                                                                 targetBuffer,
                                                                 sizeof targetBuffer,
                                                                 signalNumber,
                                                                 strsignal(signalNumber));

                swimps::log::write_to_log(swimps::log::LogLevel::Debug,
                                    targetBuffer,
                                    bytesWritten);
            }


            int signalToSend = 0;

            switch(signalNumber) {
            case SIGTRAP:
                break;
            default:
                signalToSend = signalNumber;
                break;
            }

            if (ptrace(PTRACE_CONT, childPid, 0 /* ignored */, signalToSend) == -1) {
                char targetBuffer[128] = { 0 };
                const char formatBuffer[] = "ptrace(PTRACE_CONT) failed, errno %d (%s).";
                const size_t bytesWritten = swimps_format_string(formatBuffer,
                                                                 strlen(formatBuffer),
                                                                 targetBuffer,
                                                                 sizeof targetBuffer,
                                                                 errno,
                                                                 strerror(errno));

                swimps::log::write_to_log(swimps::log::LogLevel::Debug,
                                    targetBuffer,
                                    bytesWritten);

                return SWIMPS_ERROR_PTRACE_FAILED;
            }
        }
    }
}
