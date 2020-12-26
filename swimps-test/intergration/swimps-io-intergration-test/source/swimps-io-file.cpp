#include "swimps-intergration-test.h"
#include "swimps-io-file.h"

#include <filesystem>

using swimps::io::File;
using swimps::container::Span;

SCENARIO("swimps::io::File", "[swimp-io]") {
    GIVEN("A temp file.") {
        const char prefix[] = "swimps::io::File_test";
        auto file = File::create_temporary(prefix, File::Permissions::ReadWrite);

        WHEN("getPath() is called.") {
            const auto path = file.getPath();
            THEN("It starts with the prefix requested.") {
                REQUIRE(std::filesystem::path(std::string(&path[0], path.current_size())).filename().string().starts_with(prefix));
            }
        }

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
