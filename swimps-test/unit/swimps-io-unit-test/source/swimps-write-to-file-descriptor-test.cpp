#include "swimps-test.h"
#include "swimps-io.h"

#include <cstdio>
#include <unistd.h>

SCENARIO("swimps::io::write_to_file_descriptor", "[swimps-io]") {
    GIVEN("An empty file and 4 bytes of non-zero data to be written to it.") {
        char targetFileNameBuffer[] = "/tmp/swimps::io::write_to_file_descriptor-test-XXXXXX";
        const int targetFileDescriptor = mkstemp(targetFileNameBuffer);

        REQUIRE(targetFileDescriptor != -1);

        const char sourceBuffer[] = { 1, 2, 3, 4 };

        WHEN("The data is written to the file.") {
            const auto bytesWritten = swimps::io::write_to_file_descriptor(
                sourceBuffer,
                targetFileDescriptor
            );

            THEN("The function returns that it wrote 4 bytes.") {
                REQUIRE(bytesWritten == 4);
            }

            WHEN("The file is read back") {

                // Larger than should be needed so that if there's an error
                // and more than 4 bytes get written, we can see it.
                char fileReadBuffer[8] = {};

                // Seek to start of file before reading.
                lseek(targetFileDescriptor, 0, SEEK_SET);

                const ssize_t bytesRead = read(targetFileDescriptor, fileReadBuffer, sizeof fileReadBuffer);

                THEN("It contains only the requested bytes") {
                    REQUIRE(bytesRead == 4);
                    REQUIRE(fileReadBuffer[0] == 1);
                    REQUIRE(fileReadBuffer[1] == 2);
                    REQUIRE(fileReadBuffer[2] == 3);
                    REQUIRE(fileReadBuffer[3] == 4);
                }

                REQUIRE(close(targetFileDescriptor) == 0);
            }
        }
    }
}

