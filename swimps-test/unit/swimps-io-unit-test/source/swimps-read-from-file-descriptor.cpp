#include "swimps-unit-test.h"
#include "swimps-io-file.h"

#include <unistd.h>

using swimps::io::File;

SCENARIO("swimps::io::File::read", "[swimps-io]") {
    // TODO: this is more of an intergration test.
    GIVEN("A file with some data in it.") {
        char targetFileNameBuffer[] = "/tmp/swimps::io::read_from_file_descriptor-test-XXXXXX";
        const int targetFileDescriptor = mkstemp(targetFileNameBuffer);
        File targetFile{targetFileDescriptor, targetFileNameBuffer};

        {
            const int32_t data = 42;
            targetFile.write(data);
        }

        WHEN("It is read back as an integer.") {
            REQUIRE(targetFile.seekToStart());

            int32_t myInt = 0;
            const bool succeeded = targetFile.read(myInt);

            THEN("The read succeeds.") {
                REQUIRE(succeeded);
            }

            THEN("The data is read correctly.") {
                REQUIRE(myInt == 42);
            }
        }
    }
}
