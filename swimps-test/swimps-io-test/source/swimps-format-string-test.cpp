#include "swimps-test.h"
#include "swimps-io.h"

SCENARIO("swimps_format_string", "[swimps-io]") {
    GIVEN("A zero-initialised target buffer of 8 bytes.") {
        char targetBuffer[8] = { };

        WHEN("A string with no format specifiers or null terminator is written into it.") {
            const char formatBuffer[] = { 'a', 'b', 'c', 'd' };
            const size_t bytesWritten = swimps_format_string(formatBuffer,
                                                             sizeof formatBuffer,
                                                             targetBuffer,
                                                             sizeof targetBuffer);

            THEN("The function returns that it has written 4 bytes.") {
                REQUIRE(bytesWritten == 4);
            }

            THEN("The provided characters are present in the target buffer.") {
                REQUIRE(targetBuffer[0] == 'a');
                REQUIRE(targetBuffer[1] == 'b');
                REQUIRE(targetBuffer[2] == 'c');
                REQUIRE(targetBuffer[3] == 'd');
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
