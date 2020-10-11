#include "swimps-test.h"
#include "swimps-io.h"

SCENARIO("swimps::io::write_to_buffer", "[swimps-io]") {
    GIVEN("A zero-initialised target buffer of 8 bytes.") {
        char targetBuffer[8] = { };

        // Not really a test, but an assumption of the test setup.
        // If violated, all bets are off anyway.
        REQUIRE(std::all_of(std::begin(targetBuffer), std::end(targetBuffer), [](const char byte) { return byte == 0; }));

        WHEN("4 bytes of non-zero data are written in.") {
            const char sourceBuffer[4] = {1, 2, 3, 4};

            const auto bytesWritten = swimps::io::write_to_buffer(
                sourceBuffer,
                sizeof sourceBuffer,
                targetBuffer,
                sizeof targetBuffer
            );
 
            THEN("The function returns that it has written 4 bytes.") {
                REQUIRE(bytesWritten == 4);
            }

            THEN("The 4 bytes are present in the target buffer.") {
                REQUIRE(targetBuffer[0] == 1);
                REQUIRE(targetBuffer[1] == 2);
                REQUIRE(targetBuffer[2] == 3);
                REQUIRE(targetBuffer[3] == 4);
            }

            THEN("The remaining 4 bytes are unchanged (still 0).") {
                REQUIRE(targetBuffer[4] == 0);
                REQUIRE(targetBuffer[5] == 0);
                REQUIRE(targetBuffer[6] == 0);
                REQUIRE(targetBuffer[7] == 0);
            }
        }
    }
}

