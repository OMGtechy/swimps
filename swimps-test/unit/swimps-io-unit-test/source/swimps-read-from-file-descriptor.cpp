#include "swimps-unit-test.h"
#include "swimps-io.h"

#include <unistd.h>

SCENARIO("swimps::io::read_from_file_descriptor", "[swimps-io]") {
    // TODO: this is more of an intergration test.
    GIVEN("A file with some data in it.") {
        char targetFileNameBuffer[] = "/tmp/swimps::io::read_from_file_descriptor-test-XXXXXX";
        const int targetFileDescriptor = mkstemp(targetFileNameBuffer);

        REQUIRE(targetFileDescriptor != -1);

        {
            const int32_t data = 42;
            swimps::io::write_to_file_descriptor(data, targetFileDescriptor);
        }

        WHEN("It is read back as an integer.") {
            REQUIRE(lseek(targetFileDescriptor, 0, SEEK_SET) == 0);

            int32_t myInt = 0;
            const bool succeeded = swimps::io::read_from_file_descriptor(targetFileDescriptor, myInt);

            THEN("The read succeeds.") {
                REQUIRE(succeeded);
            }

            THEN("The data is read correctly.") {
                REQUIRE(myInt == 42);
            }
        }
    }
}
