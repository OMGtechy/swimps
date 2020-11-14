#include "swimps-profile.h"
#include "swimps-io.h"
#include "swimps-log.h"

#include <cerrno>
#include <cstring>

#include <sys/wait.h>
#include <sys/ptrace.h>

swimps::error::ErrorCode swimps_profile_parent(const pid_t childPid) {
    while(1) {
        int status = 0;
        waitpid(childPid, &status, 0);

        if (WIFEXITED(status)) {
            const auto exitCode = WEXITSTATUS(status);
            const char message[] = "Child process exited with code %d.";

            swimps::log::format_and_write_to_log<256>(
                swimps::log::LogLevel::Debug,
                { message, strlen(message) },
                exitCode
            );

            return exitCode == 0 ? swimps::error::ErrorCode::None
                                 : swimps::error::ErrorCode::ChildProcessHasNonZeroExitCode;
        }

        if (WIFSIGNALED(status)) {
            const char message[] = "Child process exited due to a signal.";
            swimps::log::write_to_log(
                swimps::log::LogLevel::Debug,
                { message, strlen(message) }
            );

            return swimps::error::ErrorCode::ChildProcessExitedDueToSignal;
        }

        if (WIFSTOPPED(status)) {
            const int signalNumber = WSTOPSIG(status);

            {
                char targetBuffer[128] = { 0 };
                const char formatBuffer[] = "Child process stopped due to signal %d (%s).";
                const size_t bytesWritten = swimps::io::format_string(
                    formatBuffer,
                    targetBuffer,
                    signalNumber,
                    strsignal(signalNumber)
                );

                swimps::log::write_to_log(
                    swimps::log::LogLevel::Debug,
                    { targetBuffer, bytesWritten }
                );
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
                const size_t bytesWritten = swimps::io::format_string(
                    formatBuffer,
                    targetBuffer,
                    errno,
                    strerror(errno)
                );

                swimps::log::write_to_log(
                    swimps::log::LogLevel::Debug,
                    { targetBuffer, bytesWritten }
                );

                return swimps::error::ErrorCode::PtraceFailed;
            }
        }
    }
}
