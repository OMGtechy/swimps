#pragma once

#include <optional>
#include <string>
#include <variant>

#include "cxxopts.hpp"

#include "swimps-log.h"

namespace swimps::option {
    //!
    //! \brief  Represents a configuration of swimps.
    //!
    struct Options {
        std::string targetProgram;
        swimps::log::LogLevel logLevel;
    };

    //!
    //! \brief  Parses a swimps command line.
    //!
    //! \param[in]  argc  The argc given main().
    //! \param[in]  argv  The argv given to main().
    //!
    //! \returns  The parsed command line options.
    //!
    //! \note  This function is *not* async signal safe.
    //!
    Options parse_command_line(
        int argc,
        char* argv[]);
}
