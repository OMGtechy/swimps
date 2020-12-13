#include "swimps-unit-test.h"
#include "swimps-io.h"

#include <cstring>

SCENARIO("swimps::io::format_string", "[swimps-io]") {
    GIVEN("A zero-initialised target buffer of 8 bytes.") {
        char targetBuffer[8] = { };

        WHEN("A 4 byte string with no format specifiers or null terminator is written into it.") {
            const char formatBuffer[] = { 'a', 'b', 'c', 'd' };
            const size_t bytesWritten = swimps::io::format_string(
                formatBuffer,
                targetBuffer
            );

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
            const size_t bytesWritten = swimps::io::format_string(
                formatBuffer,
                targetBuffer
            );

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

    GIVEN("A target buffer of 3 bytes, where each byte has been initialised as 120.") {
        char targetBuffer[3];
        memset(targetBuffer, 120, sizeof targetBuffer);

        WHEN("A 2 byte string with a format specifier, a targetBufferSize of 2, a single digit arg and no null terminator is given to it.") {
            const char formatBuffer[] = { '%' };
            const size_t bytesWritten = swimps::io::format_string(
                formatBuffer,
                swimps::container::Span<char>(targetBuffer, 2),
                7)
            ;

            THEN("The function returns that it has written 1 byte.") {
                REQUIRE(bytesWritten == 1);
            }

            THEN("The formatted character is present in the target buffer.") {
                REQUIRE(targetBuffer[0] == '7');
            }

            THEN("The remaining bytes are unchanged (still 120).") {
                REQUIRE(targetBuffer[1] == 120);
                REQUIRE(targetBuffer[2] == 120);
            }
        }
    }

    GIVEN("A target buffer of 3 bytes, where each byte has been initialised as 24.") {
        char targetBuffer[3];
        memset(targetBuffer, 24, sizeof targetBuffer);

        WHEN("A 2 byte string with a format specifier with no null terminator, a targetBufferSize of 2 and a negative single digit arg is given to it.") {
            const char formatBuffer[] = { '%' };
            const size_t bytesWritten = swimps::io::format_string(
                formatBuffer,
                swimps::container::Span<char>(targetBuffer, 2),
                -1
            );

            THEN("The function returns that it has written 2 bytes.") {
                REQUIRE(bytesWritten == 2);
            }

            THEN("The formatted characters are present in the target buffer.") {
                REQUIRE(targetBuffer[0] == '-');
                REQUIRE(targetBuffer[1] == '1');
            }

            THEN("The remaining bytes are unchanged (still 24).") {
                REQUIRE(targetBuffer[2] == 24);
            }
        }
    }

    GIVEN("A target buffer of 2 bytes where each byte has been initialised as 5.") {
        char targetBuffer[2];
        memset(targetBuffer, 5, sizeof targetBuffer);

        WHEN("A 2 byte string with a format specifier, a targetBufferSize of 1, a negative single digit arg and no null terminator is given to it.") {
            const char formatBuffer[] = { '%' };
            const size_t bytesWritten = swimps::io::format_string(
                formatBuffer,
                swimps::container::Span<char>(targetBuffer, 1),
                -9
            );

            THEN("The function returns that it has written one byte.") {
                REQUIRE(bytesWritten == 1);
            }

            THEN("The formatted characters are present in the target buffer.") {
                REQUIRE (targetBuffer[0] == '-');
            }

            THEN("The remaining byte is unchanged (still 5).") {
                REQUIRE (targetBuffer[1] == 5);
            }
        }
    }

    GIVEN("A target buffer of 4 bytes where each byte has been initialised as 8.") {
        char targetBuffer[4];
        memset(targetBuffer, 8, sizeof targetBuffer);

        WHEN("A 2 byte string with a format specifier, a targetBufferSize of 2, a four digit arg and no null terminator is written into it.") {
            const char formatBuffer[] = { '%' };
            const size_t bytesWritten = swimps::io::format_string(
                formatBuffer,
                swimps::container::Span<char>(targetBuffer, 2),
                1234
            );

            THEN("The function returns that it has written 2 bytes.") {
                REQUIRE(bytesWritten == 2);
            }

            THEN("The provided characters are present in the first 2 bytes of the target buffer.") {
                REQUIRE(targetBuffer[0] == '1');
                REQUIRE(targetBuffer[1] == '2');
            }

            THEN("The remaining 2 bytes are unchanged (still 8).") {
                REQUIRE(targetBuffer[2] == 8);
                REQUIRE(targetBuffer[3] == 8);
            }
        }
    }

    GIVEN("A target buffer of 8 bytes where each byte has been initialised as 96.") {
        char targetBuffer[8];
        memset(targetBuffer, 96, sizeof targetBuffer);

        WHEN("A 2 byte string with a format specifier, a targetBufferSize of 4, a four digit arg and no null terminator is written into it.") {
            const char formatBuffer[] = { '%' };
            const size_t bytesWritten = swimps::io::format_string(
                formatBuffer,
                swimps::container::Span<char>(targetBuffer, 4),
                -1234
            );

            THEN("The function returns that it has written 4 bytes.") {
                REQUIRE(bytesWritten == 4);
            }

            THEN("The provided characters are present in the first 4 bytes of the target buffer.") {
                REQUIRE(targetBuffer[0] == '-');
                REQUIRE(targetBuffer[1] == '1');
                REQUIRE(targetBuffer[2] == '2');
                REQUIRE(targetBuffer[3] == '3');
            }

            THEN("The remaining 2 bytes are unchanged (still 96).") {
                REQUIRE(targetBuffer[4] == 96);
                REQUIRE(targetBuffer[5] == 96);
                REQUIRE(targetBuffer[6] == 96);
                REQUIRE(targetBuffer[7] == 96);
            }
        }
    }

    GIVEN("An uninitialised target buffer of 64 bytes.") {
        char targetBuffer[64];

        WHEN("A 2 byte string with a format specifier that has no null terminator and a string arg is written into it.") {
            const char formatBuffer[] = { '%' };
            const size_t bytesWritten = swimps::io::format_string(
                formatBuffer,
                targetBuffer,
                "The quick fox jumps over the lazy brown dog."
            );

            THEN("The function returns that it has written 44 bytes.") {
                REQUIRE(bytesWritten == 44);
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
            }
        }
    }

    GIVEN("A target buffer of 53 bytes where each byte has been initialised as 1.") {
        char targetBuffer[53];
        memset(targetBuffer, 1, sizeof targetBuffer);

        WHEN("A 41 byte string with three format specifiers, a null terminator, two string args and an integer arg.") {
            const char formatBuffer[] = "%, my name is % and I am % years old.";
            const size_t bytesWritten = swimps::io::format_string(
                formatBuffer,
                targetBuffer,
                "Greetings",
                "Dave",
                30
            );

            THEN("The function returns that it has written 50 bytes.") {
                REQUIRE(bytesWritten == 50);
            }

            THEN("The formatted character is present in the target buffer.") {
                const char stringToTestAgainst[] = "Greetings, my name is Dave and I am 30 years old.";
                REQUIRE(strncmp(targetBuffer, stringToTestAgainst, strlen(stringToTestAgainst)) == 0);
            }

            THEN("A null terminator is present after the string") {                                                                   
                REQUIRE(targetBuffer[49] == '\0');
            }

            THEN("The remaining 3 bytes are unchanged (still 1).") {
                REQUIRE(targetBuffer[50] == 1);
                REQUIRE(targetBuffer[51] == 1);
                REQUIRE(targetBuffer[52] == 1);
            }
        }
    }

    GIVEN("errno is 0 and a zero-iniitalised target buffer of 12 bytes") {
        errno = 0;
        char targetBuffer[12];
        memset(targetBuffer, 42, sizeof targetBuffer);

        WHEN("A format specifier is passed alongside errno.") {
            const char formatBuffer[] = "errno %.";
            const size_t bytesWritten = swimps::io::format_string(
                formatBuffer,
                targetBuffer,
                errno
            );

            THEN("The number of bytes claimed to be written is as expected.") {
                REQUIRE(bytesWritten == 9);
            }

            THEN("errno's value is unchanged.") {
                REQUIRE(errno == 0);
            }

            THEN("The resulting string is correct.") {
                REQUIRE(targetBuffer[0] == 'e');
                REQUIRE(targetBuffer[1] == 'r');
                REQUIRE(targetBuffer[2] == 'r');
                REQUIRE(targetBuffer[3] == 'n');
                REQUIRE(targetBuffer[4] == 'o');
                REQUIRE(targetBuffer[5] == ' ');
                REQUIRE(targetBuffer[6] == '0');
                REQUIRE(targetBuffer[7] == '.');
                REQUIRE(targetBuffer[8] == '\0');
            }

            THEN("The remaining bytes are unchanged.") {
                REQUIRE(targetBuffer[9]  == 42);
                REQUIRE(targetBuffer[10] == 42);
                REQUIRE(targetBuffer[11] == 42);
            }
        }
    }

    GIVEN("A zero-initialised buffer of 14 bytes.") {
        char targetBuffer[14] = { };

        WHEN("Four format specifiers are passed alongside const and non-const char buffers.") {
            const char arg1[] = "Hello";
            const char arg2[] = " Wor";
            char arg3[] = "ld";
            char arg4[] = "!";

            const char formatBuffer[] = "%%%%";
            const size_t bytesWritten = swimps::io::format_string<const char*, const char* const, char*, char* const>(
                formatBuffer,
                targetBuffer,
                arg1,
                arg2,
                arg3,
                arg4
            );

            THEN("The number of bytes claimed to be written as expected.") {
                REQUIRE(bytesWritten == 13);
            }

            THEN("The formatted string is correct.") {
                REQUIRE(targetBuffer[0] == 'H');
                REQUIRE(targetBuffer[1] == 'e');
                REQUIRE(targetBuffer[2] == 'l');
                REQUIRE(targetBuffer[3] == 'l');
                REQUIRE(targetBuffer[4] == 'o');
                REQUIRE(targetBuffer[5] == ' ');
                REQUIRE(targetBuffer[6] == 'W');
                REQUIRE(targetBuffer[7] == 'o');
                REQUIRE(targetBuffer[8] == 'r');
                REQUIRE(targetBuffer[9] == 'l');
                REQUIRE(targetBuffer[10] == 'd');
                REQUIRE(targetBuffer[11] == '!');
                REQUIRE(targetBuffer[12] == 0);
            }

            THEN("The remaing bytes are untouched.") {
                REQUIRE(targetBuffer[13] == 0);
            }
        }
    }

    GIVEN("A target buffer of 4 initialised bytes.") {
        char targetBuffer[] = { 0, 1, 2, 3 };
        static_assert(sizeof(targetBuffer) == 4);

        WHEN("A long format string with no format specifiers is provided and only 2 bytes of the target buffer are provided.") {
            const char formatBuffer[] = "This is too big.";
            const size_t bytesWritten = swimps::io::format_string(
                formatBuffer,
                { targetBuffer, 2 }
            );

            THEN("The bytes written match the number of bytes provided.") {
                REQUIRE(bytesWritten == 2);
            }

            THEN("The bytes that would have fit have been copied over.") {
                REQUIRE(targetBuffer[0] == 'T');
                REQUIRE(targetBuffer[1] == 'h');
            }

            THEN("The remaining bytes are unchanged.") {
                REQUIRE(targetBuffer[2] == 2);
                REQUIRE(targetBuffer[3] == 3);
            }
        }
    }
}
