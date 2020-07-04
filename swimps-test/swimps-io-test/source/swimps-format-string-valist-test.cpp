#include "swimps-test.h"
#include "swimps-io.h"

#include <cstring>

namespace {
    template <typename Callable>
    decltype(auto) call_with_valist(Callable callable, ...) {
        va_list varargs;
        va_start (varargs, callable);

        const auto result = callable(varargs);

        va_end(varargs);

        return result;
    }
}

SCENARIO("swimps_format_string_valist", "[swimps-io]") {
    GIVEN("A zero-initialised target buffer of 16 bytes.") {
        char targetBuffer[16] = { };

        WHEN("A string with an integer and string format specified is given to it.") {
            const char formatBuffer[] = "D: %d, S: %s!";

            const size_t bytesWritten = call_with_valist(
                [&formatBuffer, &targetBuffer](va_list varargs) {
                    return swimps_format_string_valist(
                        formatBuffer,
                        sizeof formatBuffer,
                        targetBuffer,
                        sizeof targetBuffer,
                        varargs
                    );
                },
                42,
                "Wow"
            );

            THEN("The function returns that it has written the correct number of bytes.") {
                REQUIRE(bytesWritten == 15);
            }

            THEN("The provided characters are present in the target buffer.") {
                REQUIRE(targetBuffer[0]  == 'D');
                REQUIRE(targetBuffer[1]  == ':');
                REQUIRE(targetBuffer[2]  == ' ');
                REQUIRE(targetBuffer[3]  == '4');
                REQUIRE(targetBuffer[4]  == '2');
                REQUIRE(targetBuffer[5]  == ',');
                REQUIRE(targetBuffer[6]  == ' ');
                REQUIRE(targetBuffer[7]  == 'S');
                REQUIRE(targetBuffer[8]  == ':');
                REQUIRE(targetBuffer[9]  == ' ');
                REQUIRE(targetBuffer[10] == 'W');
                REQUIRE(targetBuffer[11] == 'o');
                REQUIRE(targetBuffer[12] == 'w');
                REQUIRE(targetBuffer[13] == '!');
                REQUIRE(targetBuffer[14] == '\0');
            }

            THEN("The remaining bytes are unchanged (still 0).") {
                REQUIRE(targetBuffer[15] == 0);
            }
        }
    }
}
