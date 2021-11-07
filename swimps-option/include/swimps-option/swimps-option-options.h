#pragma once

#include <string>
#include <vector>

#include "swimps-log/swimps-log.h"

namespace swimps::option {
    //!
    //! \brief  Represents a configuration of swimps.
    //!
    struct Options {
        bool help = false;
        bool tui = true;
        bool ptrace = true;
        bool load = false;
        swimps::log::LogLevel logLevel = swimps::log::LogLevel::Info;
        double samplesPerSecond = 1.0;
        std::string targetTraceFile;

        std::string targetProgram;
        std::vector<std::string> targetProgramArgs;

        //!
        //! \brief  Turns options into a string.
        //!
        //! \returns  A string representation of the options.
        //!
        //! \note  The original purpose of this was so that
        //!        it could be passed via an environment variable.
        //!
        //! \note  This function is *not* async signal safe.
        //!
        std::string toString() const;

        //!
        //! \brief  Turns a string into options.
        //!
        //! \param[in]  string  The string to convert.
        //!
        //! \returns  The options the string represents.
        //!
        //! \note  This function is *not* async signal safe.
        //!
        //! \note  If the option string is malformed, behaviour is undefined.
        //!
        static Options fromString(std::string string);

        bool operator==(const Options&) const = default;
    };
}
