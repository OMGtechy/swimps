#include "swimps-preload/swimps-preload.h"

#include <fstream>
#include <string>

#include <linux/limits.h>
#include <unistd.h>

#include <signalsafe/string.hpp>

#include "swimps-assert/swimps-assert.h"

using signalsafe::string::format;

using swimps::trace::ProcMaps;

ProcMaps swimps::preload::get_proc_maps() {
    char pathTargetBuffer[PATH_MAX + 1 /* null terminator */] = { };

    format(
        "/proc/%/maps",
        pathTargetBuffer,
        getpid()
    );

    ProcMaps procMaps;
    std::ifstream procMapsFile(pathTargetBuffer);

    do {
        std::string rangeString;
        std::string dummy;

        procMapsFile >> rangeString;
        std::getline(procMapsFile, dummy); // ignore the rest of the line

        const auto hyphenOffset = rangeString.find('-');

        if (hyphenOffset == std::string::npos) {
            break;
        }

        const auto rangeStartString = rangeString.substr(0, hyphenOffset);
        const auto rangeEndString = rangeString.substr(hyphenOffset + 1); 

        procMaps.entries.push_back({
            { std::stoull(rangeStartString, 0, 16), std::stoull(rangeEndString, 0, 16) }
        });
    } while(! procMapsFile.eof());

    return procMaps;
}
