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
        ChildProcessHasNonZeroExitCode
    };
}
