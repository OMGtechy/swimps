#include "swimps-unit-test.h"
#include "swimps-io/swimps-io.h"

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
                targetBuffer
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

    GIVEN("A zero-initialised target buffer of 9 bytes.") {
        char targetBuffer[9] = { };

        WHEN("When 9 bytes of data are given to it, but only 6 bytes of the target buffer are provided.") {
            const char sourceBuffer[] = {123, 5, 42, 0, 0, 3, 18, 21, 4};
            static_assert(sizeof(sourceBuffer) == 9);

            const auto bytesWritten = swimps::io::write_to_buffer(
                sourceBuffer,
                { targetBuffer, 6 }
            );

            THEN("The function returns that it has written 6 bytes.") {
                REQUIRE(bytesWritten == 6);
            }

            THEN("The first 6 bytes of the target buffer contain the correct data.") {
                REQUIRE(targetBuffer[0] == 123);
                REQUIRE(targetBuffer[1] == 5);
                REQUIRE(targetBuffer[2] == 42);
                REQUIRE(targetBuffer[3] == 0);
                REQUIRE(targetBuffer[4] == 0);
                REQUIRE(targetBuffer[5] == 3);
            }

            THEN("The remaining bytes of the target buffer are unchanged.") {
                REQUIRE(targetBuffer[6] == 0);
                REQUIRE(targetBuffer[7] == 0);
                REQUIRE(targetBuffer[8] == 0);
            }
        }
    }
}

