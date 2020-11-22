#include "swimps-test.h"
#include "swimps-log.h"

#include <cstring>

SCENARIO("swimps::log::format_message", "[swimps-log]") {
    GIVEN("A simple message and a zero-initialised target buffer with plenty of extra room.") {
        char targetBuffer[2048] =  { };
        const char message[] = "This is a simple message.";

        WHEN("It is formatted.") {

            const auto bytesWritten = swimps::log::format_message(
                swimps::log::LogLevel::Debug,
                { message, sizeof message - 1 /* remove null terminator */ },
                targetBuffer
            );

            THEN("The number of bytes written matches the amount of formatted characters.") {
                REQUIRE(bytesWritten != 0);

                // This is safe since the target buffer is zero-initialised, effectively making it null-terminated.
                REQUIRE(targetBuffer[bytesWritten] == '\0');
                REQUIRE(targetBuffer[bytesWritten - 1] != '\0');
            }
        }
    }
}
