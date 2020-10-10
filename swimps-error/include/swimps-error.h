#pragma once

namespace swimps::error {
    enum class ErrorCode {
        None,
        NullParameter,
        InvalidParameter,
        ForkFailed,
        PtraceFailed,
        ReadlinkFailed,
        ExecveFailed
    };
}
