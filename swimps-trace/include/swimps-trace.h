#pragma once

#include "swimps-time.h"

#include <vector>

namespace swimps::trace {
    // Signed integers chosen because it's easier to spot errors when they overflow.

    // I highly doubt anyone will need anywhere near ~2^31 stack frames, so anything
    // overflowing here is probably bad data. I have, however, seen stack traces of 2^16 before.
    // 32-bits are chosen simply because it's the next power of 2 after 16.
    using stack_frame_count_t = int32_t;

    // It's feasible for a run to gather 2^31 samples, but 2^63 will take
    // hundreds of years even at high sample rates...
    using sample_count_t = int64_t;

    // We could, in theory, have unique backtraces for each sample.
    using backtrace_id_t = sample_count_t;
    using backtrace_count_t = sample_count_t;

    struct Backtrace {
        backtrace_id_t id;
        std::vector<char*> stackFrames;
    };

    struct Sample {
        backtrace_id_t backtraceID;
        swimps::time::TimeSpecification timestamp;
    };

    struct Trace {
        std::vector<Sample> samples;
        std::vector<Backtrace> backtraces;
    };
}
