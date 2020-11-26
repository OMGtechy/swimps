#pragma once

namespace swimps::error {
    enum class ErrorCode : int {
        None = 0,
        NullParameter,
        InvalidParameter,
        ForkFailed,
        PtraceFailed,
        ReadlinkFailed,
        ExecveFailed,
        OpenFailed,
        ChildProcessHasNonZeroExitCode,
        ChildProcessExitedDueToSignal,
        CommandLineParseFailed,
        ReadSampleFailed,
        ReadBacktraceFailed,
        UnknownEntryKind,
        EndOfFile
    };
}
