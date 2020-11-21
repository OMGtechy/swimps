#include "swimps-profile.h"
#include "swimps-option.h"
#include "swimps-log.h"

int main(int argc, char** argv) {
    const auto options = swimps::option::parse_command_line(
        argc,
        argv
    );

    swimps::log::setLevelToLog(options.logLevel);
    return static_cast<int>(swimps::profile::start(options));
}
