#include "swimps-unit-test.h"
#include "swimps-io-file.h"

#include <cstdio>
#include <unistd.h>

using swimps::io::File;

SCENARIO("swimps::io::File::write", "[swimps-io]") {
    GIVEN("An empty file and 4 bytes of non-zero data to be written to it.") {
        char targetFileNameBuffer[] = "/tmp/swimps::io::write_to_file_descriptor-test-XXXXXX";
        const int targetFileDescriptor = mkstemp(targetFileNameBuffer);
        File targetFile(targetFileDescriptor, targetFileNameBuffer);

        const char sourceBuffer[] = { 1, 2, 3, 4 };

        WHEN("The data is written to the file.") {
            const auto bytesWritten = targetFile.write(sourceBuffer);

            THEN("The function returns that it wrote 4 bytes.") {
                REQUIRE(bytesWritten == 4);
            }

            WHEN("The file is read back") {

                // Larger than should be needed so that if there's an error
                // and more than 4 bytes get written, we can see it.
                char fileReadBuffer[8] = {};

                // Seek to start of file before reading.
                // TODO: Should this be merged with read into a File test suite?
                // TODO: Is this more of an intergration test now?
                REQUIRE(targetFile.seekToStart());

                const ssize_t bytesRead = read(targetFileDescriptor, fileReadBuffer, sizeof fileReadBuffer);

                THEN("It contains only the requested bytes") {
                    REQUIRE(bytesRead == 4);
                    REQUIRE(fileReadBuffer[0] == 1);
                    REQUIRE(fileReadBuffer[1] == 2);
                    REQUIRE(fileReadBuffer[2] == 3);
                    REQUIRE(fileReadBuffer[3] == 4);
                }

                REQUIRE(targetFile.close());
            }
        }
    }
}

