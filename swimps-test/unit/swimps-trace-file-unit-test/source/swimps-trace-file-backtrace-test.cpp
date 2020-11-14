#include "swimps-test.h"

#include <cstdio>
#include <array>

#include "swimps-io.h"
#include "swimps-trace-file.h"
#include "swimps-container.h"

using namespace swimps::trace;
using namespace swimps::trace::file;

// TODO: this is more of an intergration test than unit ... make a folder for such things and move it there.

SCENARIO("swimps::trace::file::read_backtrace", "[swimps-trace-file]") {
    GIVEN("A temp file.") {
        char targetFileNameBuffer[] = "/tmp/swimps::trace::file::read_backtrace-test-XXXXXX";
        const int targetFileDescriptor = mkstemp(targetFileNameBuffer);

        std::array<char, 2048> data = { };
        auto dataSpan = swimps::container::Span<char>(data);

        // 1) Backtrace ID (8 bytes)
        static_assert(sizeof(backtrace_id_t) == 8);

        constexpr backtrace_id_t backtraceID = 9;
        memcpy(&dataSpan[0], &backtraceID, sizeof(backtrace_id_t));

        dataSpan += sizeof(backtrace_id_t);

        // 2) Stack frame counter (4 bytes)
        static_assert(sizeof(stack_frame_count_t) == 4);

        std::vector<StackFrame> stackFrames;
        stackFrames.push_back(StackFrame{"Hi", static_cast<mangled_function_name_length_t>(strlen("Hi")), 42});
        stackFrames.push_back(StackFrame{"there", static_cast<mangled_function_name_length_t>(strlen("there")), 12});
        stackFrames.push_back(StackFrame{"backtrace", static_cast<mangled_function_name_length_t>(strlen("backtrace")), 17});

        {
            const stack_frame_count_t stackFrameCount = stackFrames.size();
            memcpy(&dataSpan[0], &stackFrameCount, sizeof(stackFrameCount));
            dataSpan += sizeof(stackFrameCount);
        }

        // Then, for each backtrace
        for (auto& stackFrame : stackFrames) {
            // 1) Mangled function name length (4 bytes)
            static_assert(sizeof(mangled_function_name_length_t) == 4);
            memcpy(&dataSpan[0], &stackFrame.mangledFunctionNameLength, sizeof(stackFrame.mangledFunctionNameLength));
            dataSpan += sizeof(mangled_function_name_length_t);

            // 2) Mangled function name (variable number of bytes, determined by length written)
            memcpy(&dataSpan[0], &stackFrame.mangledFunctionName, stackFrame.mangledFunctionNameLength);
            dataSpan += stackFrame.mangledFunctionNameLength;

            // 3) Offset (8 bytes)
            static_assert(sizeof(offset_t) == 8);
            memcpy(&dataSpan[0], &stackFrame.offset, sizeof stackFrame.offset);
            dataSpan += sizeof(offset_t);
        }

        WHEN("A valid backtrace is written to it.") {
            REQUIRE(swimps::io::write_to_file_descriptor(
                data,
                targetFileDescriptor
            ) == data.size());

            AND_WHEN("It is read back.") {
                REQUIRE(lseek(targetFileDescriptor, 0, SEEK_SET) == 0);

                const auto backtrace = swimps::trace::file::read_backtrace(targetFileDescriptor);

                THEN("A valid backtrace is returned.") {
                    REQUIRE(backtrace);

                    AND_THEN("The backtrace has the correct number of stack frames.") {
                        REQUIRE(backtrace->stackFrameCount == 3);
                        REQUIRE(stackFrames.size() == 3);

                        AND_THEN("The stack frames are equal to those written.") {
                            REQUIRE(stackFrames[0] == backtrace->stackFrames[0]);
                            REQUIRE(stackFrames[1] == backtrace->stackFrames[1]);
                            REQUIRE(stackFrames[2] == backtrace->stackFrames[2]);
                        }
                    }

                    AND_THEN("The backtrace ID is correct.") {
                        REQUIRE(backtrace->id == backtraceID);
                    }
                }
            }
        }
    }
}
