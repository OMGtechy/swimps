#pragma once

#include "swimps-time.h"

// Signed integers chosen because it's easier to spot errors when they overflow.

// I highly doubt anyone will need anywhere near ~2^31 stack frames, so anything
// overflowing here is probably bad data. I have, however, seen stack traces of 2^16 before.
// 32-bits are chosen simply because it's the next power of 2 after 16.
typedef int32_t swimps_stack_frame_count_t;

// It's feasible for a run to gather 2^31 samples, but 2^63 will take
// hundreds of years even at high sample rates...
typedef int64_t swimps_sample_count_t;

// We could, in theory, have unique backtraces for each sample.
typedef swimps_sample_count_t swimps_backtrace_id_t;
typedef swimps_sample_count_t swimps_backtrace_count_t;

typedef struct swimps_backtrace {
    swimps_backtrace_id_t id;
    char** stackFrames;
    swimps_stack_frame_count_t stackFrameCount;
} swimps_backtrace_t;

typedef struct swimps_sample {
    swimps_backtrace_id_t backtraceID;
    swimps::time::TimeSpecification timestamp;
} swimps_sample_t;

typedef struct swimps_trace {
    swimps_sample_t* samples;
    swimps_sample_count_t sampleCount;
    swimps_backtrace_t* backtraces;
    swimps_backtrace_count_t backtraceCount;
} swimps_trace_t;

