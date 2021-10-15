#include "swimps-intergration-test.h"

#include <filesystem>

#include <signalsafe/file.hpp>

#include "swimps-dwarf/swimps-dwarf.h"
#include "swimps-io/swimps-io-file.h"

using signalsafe::File;

using swimps::dwarf::DwarfInfo;
using swimps::io::format_string;

SCENARIO("swimps::dwarf::DwarfInfo, swimps::io::format_string", "[swimps-dwarf]") {
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

        auto executableFile = File::open_existing({ testDataPath.c_str(), testDataPath.length() }, File::Permissions::ReadOnly);

        WHEN("Its debug information is loaded.") {
            DwarfInfo dwarfInfo(std::move(executableFile));

            // These tests were originally derived by looking at what
            // dwarfdump found in the file.

            const auto lineInfos = dwarfInfo.getLineInfos();
            const auto functionInfos = dwarfInfo.getFunctionInfos();

            THEN("Line info from cu-a.cpp can be found.") {
                const auto lineInfoIter = std::find_if(lineInfos.cbegin(), lineInfos.cend(), [](const auto& lineInfo){
                    return lineInfo.getAddress() == 0x000009dc;
                });

                REQUIRE(lineInfoIter != lineInfos.cend());
                REQUIRE(lineInfoIter->getAddress() == 0x000009dc);
                REQUIRE(lineInfoIter->getLineNumber() == 5);
                REQUIRE(lineInfoIter->getSourceFilePath()->filename() == "cu-a.cpp");
            }

            THEN("Function info from cu-a.cpp can be found.") {
                const auto functionInfoIter = std::find_if(functionInfos.cbegin(), functionInfos.cend(), [](const auto& functionInfo){
                    return functionInfo.getName() == "main"; 
                });

                REQUIRE(functionInfoIter != functionInfos.cend());
                REQUIRE(functionInfoIter->getName() == "main");
                REQUIRE(functionInfoIter->getLowPC() == 0x000009d4);
                REQUIRE(functionInfoIter->getHighPC() == 0x000009d4 + 28);
            }

            THEN("Line info from cu-b.cpp can be found.") {
                const auto lineInfoIter = std::find_if(lineInfos.cbegin(), lineInfos.cend(), [](const auto& lineInfo){
                    return lineInfo.getAddress() == 0x00000a2c;
                });

                REQUIRE(lineInfoIter != lineInfos.cend());
                REQUIRE(lineInfoIter->getAddress() == 0x00000a2c);
                REQUIRE(lineInfoIter->getLineNumber() == 7);
                REQUIRE(lineInfoIter->getSourceFilePath()->filename() == "cu-b.cpp");
            }

            THEN("Function info from cu-b.cpp can be found.") {
                const auto functionInfoIter = std::find_if(functionInfos.cbegin(), functionInfos.cend(), [](const auto& functionInfo){
                    return functionInfo.getName() == "f"; 
                });

                REQUIRE(functionInfoIter != functionInfos.cend());
                REQUIRE(functionInfoIter->getName() == "f");
                REQUIRE(functionInfoIter->getLowPC() == 0x00000a2c);
                REQUIRE(functionInfoIter->getHighPC() == 0x00000a2c + 24);
            }

            THEN("Line info from cu-c.cpp can be found.") {
                const auto lineInfoIter = std::find_if(lineInfos.cbegin(), lineInfos.cend(), [](const auto& lineInfo){
                    return lineInfo.getAddress() == 0x00000acc;
                });

                REQUIRE(lineInfoIter != lineInfos.cend());
                REQUIRE(lineInfoIter->getAddress() == 0x00000acc);
                REQUIRE(lineInfoIter->getLineNumber() == 5);
                REQUIRE(lineInfoIter->getSourceFilePath()->filename() == "cu-c.cpp");
            }

            THEN("Function info from cu-c.cpp can be found.") {
                const auto functionInfoIter = std::find_if(functionInfos.cbegin(), functionInfos.cend(), [](const auto& functionInfo){
                    return functionInfo.getName() == "h"; 
                });

                REQUIRE(functionInfoIter != functionInfos.cend());
                REQUIRE(functionInfoIter->getName() == "h");
                REQUIRE(functionInfoIter->getLowPC() == 0x00000ac0);
                REQUIRE(functionInfoIter->getHighPC() == 0x00000ac0 + 24);
            }
        }
    }
}
