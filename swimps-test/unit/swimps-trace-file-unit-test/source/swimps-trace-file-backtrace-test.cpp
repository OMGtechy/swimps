#include "swimps-unit-test.h"

#include <cstdio>
#include <array>

#include "swimps-io.h"
#include "swimps-trace-file.h"
#include "swimps-container.h"

using namespace swimps::trace;
using namespace swimps::trace::file;

using swimps::io::File;

// TODO: this is more of an intergration test than unit ... make a folder for such things and move it there.

SCENARIO("swimps::trace::file::read_backtrace", "[swimps-trace-file]") {
    GIVEN("A temp file.") {
        auto targetFile = File::create_temporary("swimps::trace::file::read_backtrace_test_", File::Permissions::ReadWrite);

        std::array<char, 2048> data = { };
        auto dataSpan = swimps::container::Span<char>(data);

        // 1) Backtrace ID (8 bytes)
        static_assert(sizeof(backtrace_id_t) == 8);

        constexpr backtrace_id_t backtraceID = 9;
        memcpy(&dataSpan[0], &backtraceID, sizeof(backtrace_id_t));

        dataSpan += sizeof(backtrace_id_t);

        // 2) Stack frame counter (4 bytes)
        static_assert(sizeof(stack_frame_count_t) == 4);

        std::vector<stack_frame_id_t> stackFrameIDs;
        stackFrameIDs.push_back(0);
        stackFrameIDs.push_back(1);
        stackFrameIDs.push_back(2);

        {
            const stack_frame_count_t stackFrameIDCount = stackFrameIDs.size();
            memcpy(&dataSpan[0], &stackFrameIDCount, sizeof(stackFrameIDCount));
            dataSpan += sizeof(stackFrameIDCount);
        }

        // Write out the stack frame IDs used (8 bytes each)
        for (auto& stackFrameID : stackFrameIDs) {
            static_assert(sizeof(stack_frame_id_t) == 8);
            memcpy(&dataSpan[0], &stackFrameID, sizeof(stackFrameID));
            dataSpan += sizeof(stack_frame_id_t);
        }

        WHEN("A valid backtrace is written to it.") {
            REQUIRE(targetFile.write(data) == data.size());

            AND_WHEN("It is read back.") {
                REQUIRE(targetFile.seekToStart());

                const auto backtrace = swimps::trace::file::read_backtrace(targetFile);

                THEN("A valid backtrace is returned.") {
                    REQUIRE(backtrace);

                    AND_THEN("The backtrace has the correct number of stack frames.") {
                        REQUIRE(backtrace->stackFrameIDCount == 3);
                        REQUIRE(stackFrameIDs.size() == 3);

                        AND_THEN("The stack frames are equal to those written.") {
                            REQUIRE(stackFrameIDs[0] == backtrace->stackFrameIDs[0]);
                            REQUIRE(stackFrameIDs[1] == backtrace->stackFrameIDs[1]);
                            REQUIRE(stackFrameIDs[2] == backtrace->stackFrameIDs[2]);
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
