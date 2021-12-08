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
        InjectLibraryFailed,
        ChildProcessHasNonZeroExitCode,
        ChildProcessExitedDueToSignal,
        CommandLineParseFailed,
        ReadProcMapsFailed,
        ReadRawSampleFailed,
        ReadSampleFailed,
        ReadBacktraceFailed,
        ReadStackFrameFailed,
        UnknownEntryKind,
        EndOfFile
    };
}
