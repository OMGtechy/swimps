#include "swimps-test.h"
#include "swimps-io.h"

SCENARIO("swimps_format_string", "[swimps-io]") {
    GIVEN("A zero-initialised target buffer of 8 bytes.") {
        char targetBuffer[8] = { };

        WHEN("A 4 byte string with no format specifiers or null terminator is written into it.") {
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

    GIVEN("An uninitialised target buffer of 12 bytes.") {
        char targetBuffer[12];

        WHEN("An 8 byte string with no format specifiers but with a null terminator is written into it.") {
            const char formatBuffer[] = "1234567"; // 8th is null terminator
            const size_t bytesWritten = swimps_format_string(formatBuffer,
                                                             sizeof formatBuffer,
                                                             targetBuffer,
                                                             sizeof targetBuffer);

            THEN("The function returns that it has written 8 bytes.") {
                REQUIRE(bytesWritten == 8);
            }

            THEN("The provided characters are present in the target buffer.") {
                REQUIRE(targetBuffer[0] == '1');
                REQUIRE(targetBuffer[1] == '2');
                REQUIRE(targetBuffer[2] == '3');
                REQUIRE(targetBuffer[3] == '4');
                REQUIRE(targetBuffer[4] == '5');
                REQUIRE(targetBuffer[5] == '6');
                REQUIRE(targetBuffer[6] == '7');
                REQUIRE(targetBuffer[7] == '\0');
            }
        }
    }

    GIVEN("An uninitialised target buffer of 2 bytes.") {
        char targetBuffer[2];

        WHEN("A 2 byte string with an integer format specifier, a single digit vararg and no null terminator is written into it.") {
            const char formatBuffer[] = { '%', 'd' };
            const size_t bytesWritten = swimps_format_string(formatBuffer,
                                                             sizeof formatBuffer,
                                                             targetBuffer,
                                                             sizeof targetBuffer,
                                                             7);

            THEN("The function returns that it has written 1 byte.") {
                REQUIRE(bytesWritten == 1);
            }

            THEN("The formatted character is present in the target buffer.") {
                REQUIRE(targetBuffer[0] == '7');
            }
        }
    }
    
    GIVEN("An uninitialised target buffer of 64 bytes.") {
        char targetBuffer[64];

        WHEN("A 2 byte string with a string format specifier, a string vararg and no null terminator is written into it.") {
            const char formatBuffer[] = { '%', 's' };
            const size_t bytesWritten = swimps_format_string(formatBuffer,
                                                             sizeof formatBuffer,
                                                             targetBuffer,
                                                             sizeof targetBuffer,
                                                             "The quick fox jumps over the lazy brown dog.");

            THEN("The function returns that it has written 45 bytes.") {
                REQUIRE(bytesWritten == 45);
            }

            THEN("The formatted character is present in the target buffer.") {
                REQUIRE(targetBuffer[0] == 'T');
                REQUIRE(targetBuffer[1] == 'h');
                REQUIRE(targetBuffer[2] == 'e');
                REQUIRE(targetBuffer[3] == ' ');
                REQUIRE(targetBuffer[4] == 'q');
                REQUIRE(targetBuffer[5] == 'u');
                REQUIRE(targetBuffer[6] == 'i');
                REQUIRE(targetBuffer[7] == 'c');
                REQUIRE(targetBuffer[8] == 'k');
                REQUIRE(targetBuffer[9] == ' ');
                REQUIRE(targetBuffer[10] == 'f');
                REQUIRE(targetBuffer[11] == 'o');
                REQUIRE(targetBuffer[12] == 'x');
                REQUIRE(targetBuffer[13] == ' ');
                REQUIRE(targetBuffer[14] == 'j');
                REQUIRE(targetBuffer[15] == 'u');
                REQUIRE(targetBuffer[16] == 'm');
                REQUIRE(targetBuffer[17] == 'p');
                REQUIRE(targetBuffer[18] == 's');
                REQUIRE(targetBuffer[19] == ' ');
                REQUIRE(targetBuffer[20] == 'o');
                REQUIRE(targetBuffer[21] == 'v');
                REQUIRE(targetBuffer[22] == 'e');
                REQUIRE(targetBuffer[23] == 'r');
                REQUIRE(targetBuffer[24] == ' ');
                REQUIRE(targetBuffer[25] == 't');
                REQUIRE(targetBuffer[26] == 'h');
                REQUIRE(targetBuffer[27] == 'e');
                REQUIRE(targetBuffer[28] == ' ');
                REQUIRE(targetBuffer[29] == 'l');
                REQUIRE(targetBuffer[30] == 'a');
                REQUIRE(targetBuffer[31] == 'z');
                REQUIRE(targetBuffer[32] == 'y');
                REQUIRE(targetBuffer[33] == ' ');
                REQUIRE(targetBuffer[34] == 'b');
                REQUIRE(targetBuffer[35] == 'r');
                REQUIRE(targetBuffer[36] == 'o');
                REQUIRE(targetBuffer[37] == 'w');
                REQUIRE(targetBuffer[38] == 'n');
                REQUIRE(targetBuffer[39] == ' ');
                REQUIRE(targetBuffer[40] == 'd');
                REQUIRE(targetBuffer[41] == 'o');
                REQUIRE(targetBuffer[42] == 'g');
                REQUIRE(targetBuffer[43] == '.');
                REQUIRE(targetBuffer[44] == '\0');
            }
        }
    }  
}
