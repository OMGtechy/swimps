#include "swimps-profile.h"
#include "swimps-io.h"
#include "swimps-log.h"

#include <cerrno>
#include <cstring>

#include <sys/wait.h>
#include <sys/ptrace.h>

swimps::error::ErrorCode swimps::profile::parent(const pid_t childPid) {
    while(1) {
        int status = 0;
        waitpid(childPid, &status, 0);

        if (WIFEXITED(status)) {
            const auto exitCode = WEXITSTATUS(status);

            swimps::log::format_and_write_to_log<256>(
                swimps::log::LogLevel::Debug,
                "Child process exited with code %d.",
                exitCode
            );

            return exitCode == 0 ? swimps::error::ErrorCode::None
                                 : swimps::error::ErrorCode::ChildProcessHasNonZeroExitCode;
        }

        if (WIFSIGNALED(status)) {
            swimps::log::write_to_log(
                swimps::log::LogLevel::Debug,
                "Child process exited due to a signal."
            );

            return swimps::error::ErrorCode::ChildProcessExitedDueToSignal;
        }

        if (WIFSTOPPED(status)) {
            const int signalNumber = WSTOPSIG(status);

            swimps::log::format_and_write_to_log<128>(
                swimps::log::LogLevel::Debug,
                "Child process stopped due to signal %d (%s).",
                signalNumber,
                strsignal(signalNumber)
            );

            int signalToSend = 0;

            switch(signalNumber) {
            case SIGTRAP:
                break;
            default:
                signalToSend = signalNumber;
                break;
            }

            if (ptrace(PTRACE_CONT, childPid, 0 /* ignored */, signalToSend) == -1) {
                swimps::log::format_and_write_to_log<128>(
                    swimps::log::LogLevel::Debug,
                    "ptrace(PTRACE_CONT) failed, errno %d (%s).",
                    errno,
                    strerror(errno)
                );

                return swimps::error::ErrorCode::PtraceFailed;
            }
        }
    }
}
