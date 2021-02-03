#include "swimps-intergration-test.h"

#include <filesystem>

#include "swimps-dwarf.h"
#include "swimps-io-file.h"

using swimps::dwarf::DwarfInfo;
using swimps::io::File;
using swimps::io::format_string;

SCENARIO("swimps::dwarf::DwarfInfo, swimps::io::format_string, swimps::io::File", "[swimps-dwarf]") {
    GIVEN("An executable with debug information across multiple CUs in it.") {
        char procExeBuffer[PATH_MAX + 1 /* null terminator */] = { };
        format_string("/proc/%/exe", procExeBuffer, getpid());

        char executablePathBuffer[PATH_MAX + 1 /* null terminator */] = { };
        const auto bytesWritten = readlink(
            procExeBuffer,
            executablePathBuffer,
            sizeof(executablePathBuffer) - 1 /* null terminator */
        );

        REQUIRE(bytesWritten > 0);
        std::filesystem::path thisExecutablePath({ &executablePathBuffer[0], static_cast<std::size_t>(bytesWritten) });

        const auto testDataPath = thisExecutablePath.parent_path().string() + "/swimps-dwarf-intergration-test/data/cu-test";

        auto executableFile = File::open({ testDataPath.c_str(), testDataPath.length() }, File::Permissions::ReadOnly);

        WHEN("Its debug information is loaded.") {
            DwarfInfo dwarfInfo(std::move(executableFile));

            // These tests were originally derived by looking at what
            // dwarfdump found in the file.

            const auto lineInfos = dwarfInfo.getLineInfos();

            THEN("Debug info from cu-a.cpp can be found.") {
                const auto lineInfoIter = std::find_if(lineInfos.cbegin(), lineInfos.cend(), [](const auto& lineInfo){
                    return lineInfo.getAddress() == 0x000009dc;
                });

                REQUIRE(lineInfoIter != lineInfos.cend());
                REQUIRE(lineInfoIter->getAddress() == 0x000009dc);
                REQUIRE(lineInfoIter->getLineNumber() == 5);
                REQUIRE(lineInfoIter->getSourceFilePath()->filename() == "cu-a.cpp");
            }

            THEN("Debug info from cu-b.cpp can be found.") {
                const auto lineInfoIter = std::find_if(lineInfos.cbegin(), lineInfos.cend(), [](const auto& lineInfo){
                    return lineInfo.getAddress() == 0x00000a2c;
                });

                REQUIRE(lineInfoIter != lineInfos.cend());
                REQUIRE(lineInfoIter->getAddress() == 0x00000a2c);
                REQUIRE(lineInfoIter->getLineNumber() == 7);
                REQUIRE(lineInfoIter->getSourceFilePath()->filename() == "cu-b.cpp");
            }

            THEN("Debug info from cu-c.cpp can be found.") {
                const auto lineInfoIter = std::find_if(lineInfos.cbegin(), lineInfos.cend(), [](const auto& lineInfo){
                    return lineInfo.getAddress() == 0x00000acc;
                });

                REQUIRE(lineInfoIter != lineInfos.cend());
                REQUIRE(lineInfoIter->getAddress() == 0x00000acc);
                REQUIRE(lineInfoIter->getLineNumber() == 5);
                REQUIRE(lineInfoIter->getSourceFilePath()->filename() == "cu-c.cpp");
            }
        }
    }
}
