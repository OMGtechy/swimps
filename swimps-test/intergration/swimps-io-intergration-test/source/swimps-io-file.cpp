#include "swimps-intergration-test.h"
#include "swimps-io-file.h"

using swimps::io::File;

SCENARIO("swimps::io::File", "[swimp-io]") {
    GIVEN("A file constructed from a file descriptor.") {
        char fileNameBuffer[] = "/tmp/swimps::io::File::write-test-XXXXXX";
        const int fileDescriptor = mkstemp(fileNameBuffer);

        File file(fileDescriptor, fileNameBuffer);

        WHEN("8 bytes of data are written into it.") {
            std::array<const char, 8> dataSource = { 120, 0, 59, 24, 2, 43, 42, 1 };
            REQUIRE(file.write(dataSource) == dataSource.size());

            AND_WHEN("seekToStart() is called.") {
                REQUIRE(file.seekToStart());

                AND_WHEN("8 bytes are read back out of it.") {
                    std::array<char, 8> dataRead = { };
                    REQUIRE(file.read(dataRead) == dataRead.size());

                    THEN("The data read and data written are equivalent.") {
                        REQUIRE(std::equal(dataSource.cbegin(), dataSource.cend(),
                                           dataRead.cbegin(), dataRead.cend()));
                    }
                }
            }
        }
    }
}
