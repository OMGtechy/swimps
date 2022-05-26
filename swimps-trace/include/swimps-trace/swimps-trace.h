#pragma once

#include <array>
#include <vector>
#include <cstring>

#include <linux/limits.h>

#include <samplerpreload/trace.hpp>

#include <signalsafe/time.hpp>

#include <signalsampler/backtrace.hpp>

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

    // If someone has a function name longer than ~2^31 characters ... well, we don't support that.
    using function_name_length_t = int32_t;

    // In theory the instruction pointer could be pretty much anywhere in the address space. 
    using address_t = uint64_t;

    // Chances are it'll never even come close to needing this many bits, but I haven't done much research on this,
    // so for now it's safer to be too big than too small.
    using offset_t = address_t;

    // 2^63 unique stack frames, if each backtrace has 256 of them, sampled every nanosecond should cover over a year of runtime.
    using stack_frame_id_t = int64_t;

    // Some build systems combine multiple source files int one giant one,
    // so ~2^31 could happen. 2^63 though, should cover anything.
    using line_number_t = int64_t;

    // I've seen some reports that file names / paths can go past PATH_MAX.
    using file_path_length_t = uint32_t;
    static_assert(PATH_MAX < std::numeric_limits<file_path_length_t>::max());

    struct StackFrame {
        StackFrame() = default;
        explicit constexpr StackFrame(const stack_frame_id_t _id, const signalsampler::instruction_pointer_t _instructionPointer)
        : id(_id),
          instructionPointer(_instructionPointer) {

        }

        stack_frame_id_t id = std::numeric_limits<stack_frame_id_t>::min();

        // TODO: could we get rid of all this extra info
        //       and just use the instruction pointer now?
        char functionName[256] = { };
        function_name_length_t functionNameLength = 0;
        offset_t offset = 0;
        address_t instructionPointer = 0;
        line_number_t lineNumber = -1;
        char sourceFilePath[PATH_MAX + 1 /* null terminator */] = { };
        file_path_length_t sourceFilePathLength = 0;

        constexpr bool isSameAs(const StackFrame& other) const noexcept {
            return id == other.id && isEquivalentTo(other);
        }

        constexpr bool isEquivalentTo(const StackFrame& other) const noexcept {
            return offset == other.offset
                && instructionPointer == other.instructionPointer
                && 0 == strncmp(&functionName[0],
                                &other.functionName[0],
                                sizeof functionName);
        }

        bool operator==(const StackFrame& other) = delete;
    };

    struct Backtrace {
        backtrace_id_t id = std::numeric_limits<backtrace_id_t>::min();
        std::vector<stack_frame_id_t> stackFrameIDs;
    };

    struct Sample {
        backtrace_id_t backtraceID = std::numeric_limits<backtrace_id_t>::min();
        signalsafe::time::TimeSpecification timestamp;
    };

    struct Trace {
        std::vector<Sample> samples;
        std::vector<Backtrace> backtraces;
        std::vector<StackFrame> stackFrames;
    };
}
