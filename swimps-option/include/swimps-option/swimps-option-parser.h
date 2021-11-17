#pragma once

#include <optional>

#include "swimps-option/swimps-option-options.h"

namespace swimps::option {
    //!
    //! \brief  Parses a swimps command line.
    //!
    //! \param[in]  argc  The argc given main().
    //! \param[in]  argv  The argv given to main().
    //!
    //! \returns  The parsed command line options, or an empty optional if an error occured.
    //!
    //! \note  This function is *not* async signal safe.
    //!
    std::optional<Options> parse_command_line(
        const int argc,
        const char* argv[]);

    //!
    //! \brief  Prints the swimps help information to stdout.
    //!
    //! \note  This function is *not* async signal safe.
    //!
    void print_help();
}
