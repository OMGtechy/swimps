#pragma once

#include "swimps-time.h"

#include <vector>
#include <cstring>

namespace swimps::trace {
    // Signed integers chosen because it's easier to spot errors when they overflow.

    // I highly doubt anyone will need anywhere near ~2^31 stack frames, so anything
    // overflowing here is probably bad data. I have, however, seen stack traces of 2^16 before.
    // 32-bits are chosen simply because it's the next power of 2 after 16.
    using stack_frame_count_t = int32_t;
    constexpr auto stack_frame_count_max = std::numeric_limits<stack_frame_count_t>::max();

    // It's feasible for a run to gather 2^31 samples, but 2^63 will take
    // hundreds of years even at high sample rates...
    using sample_count_t = int64_t;

    // We could, in theory, have unique backtraces for each sample.
    using backtrace_id_t = sample_count_t;
    using backtrace_count_t = sample_count_t;

    // If someone has a (mangled) function name longer than ~2^31 characters ... well, we don't support that.
    using mangled_function_name_length_t = int32_t;

    // Chances are it'll never even come close to needing this many bits, but I haven't done much research on this,
    // so for now it's safer to be too big than too small.
    using offset_t = int64_t;

    // 2^63 unique stack frames, if each backtrace has 256 of them, sampled every nanosecond should cover over a year of runtime.
    using stack_frame_id_t = int64_t;

    struct StackFrame {
        stack_frame_id_t id = std::numeric_limits<stack_frame_id_t>::min();

        char mangledFunctionName[256] = { };
        mangled_function_name_length_t mangledFunctionNameLength = 0;
        offset_t offset = 0;

        constexpr bool isSameAs(const StackFrame& other) const noexcept {
            return id == other.id && isEquivalentTo(other);
        }

        constexpr bool isEquivalentTo(const StackFrame& other) const noexcept {
            return offset == other.offset
                && 0 == strncmp(&mangledFunctionName[0],
                                &other.mangledFunctionName[0],
                                sizeof mangledFunctionName);
        }

        bool operator==(const StackFrame& other) = delete;
    };

    struct Backtrace {
        backtrace_id_t id = std::numeric_limits<backtrace_id_t>::min();
        stack_frame_id_t stackFrameIDs[64] = { };
        stack_frame_count_t stackFrameIDCount = 0;
    };

    struct Sample {
        backtrace_id_t backtraceID = std::numeric_limits<backtrace_id_t>::min();
        swimps::time::TimeSpecification timestamp;
    };

    struct Trace {
        std::vector<Sample> samples;
        std::vector<Backtrace> backtraces;
        std::vector<StackFrame> stackFrames;
    };
}
