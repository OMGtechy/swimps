#include "swimps-intergration-test.h"

#include <cstdio>
#include <array>

#include "swimps-io/swimps-io.h"
#include "swimps-trace-file/swimps-trace-file.h"
#include "swimps-container/swimps-container.h"

using namespace swimps::trace;

using swimps::io::File;

SCENARIO("swimps::trace::TraceFile::read_backtrace, "
         "TraceFile::create_temporary, "
         "swimps::trace::Backtrace", "[swimps-trace-file]") {
    GIVEN("A temp file.") {
        auto targetFile = TraceFile::create_temporary("swimps::trace::file::read_backtrace_test_", File::Permissions::ReadWrite);

        Backtrace writtenBacktrace;
        writtenBacktrace.id = 9;
        writtenBacktrace.stackFrameIDs[0] = 0;
        writtenBacktrace.stackFrameIDs[1] = 1;
        writtenBacktrace.stackFrameIDs[2] = 2;
        writtenBacktrace.stackFrameIDCount = 3;

        WHEN("A valid backtrace is written to it.") {
            targetFile.add_backtrace(writtenBacktrace);

            AND_WHEN("It is read back.") {
                REQUIRE(targetFile.seekToStart());

                const auto entry = targetFile.read_next_entry();

                THEN("A valid backtrace is returned.") {
                    REQUIRE(std::holds_alternative<Backtrace>(entry));

                    const auto readBacktrace = std::get<Backtrace>(entry);

                    AND_THEN("The read backtrace is equivalent to the written one.") {
                        REQUIRE(readBacktrace.id == writtenBacktrace.id);
                        REQUIRE(readBacktrace.stackFrameIDCount == writtenBacktrace.stackFrameIDCount);
                        REQUIRE(readBacktrace.stackFrameIDs == writtenBacktrace.stackFrameIDs);
                    }
                }
            }
        }
    }
}
