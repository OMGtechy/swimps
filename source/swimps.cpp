#include "swimps-profile.h"
#include "swimps-option.h"
#include "swimps-log.h"

int main(int argc, char** argv) {
    bool exceptionThrown = false;
    swimps::option::Options options;

    try {
        options = swimps::option::parse_command_line(
            argc,
            const_cast<const char**>(argv)
        );
    } catch (const std::exception& exception) {
        swimps::log::format_and_write_to_log<2048>(
            swimps::log::LogLevel::Fatal,
            "Error encountered whilst parsing command line: %s",
            exception.what()
        );

        exceptionThrown = true;
    }

    if (exceptionThrown || options.help) {
        swimps::option::print_help();
        exit(static_cast<int>(
            exceptionThrown
                ? swimps::error::ErrorCode::CommandLineParseFailed
                : swimps::error::ErrorCode::None
        ));
    }

    swimps::log::setLevelToLog(options.logLevel);
    return static_cast<int>(swimps::profile::start(options));
}
