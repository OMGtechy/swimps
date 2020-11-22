#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "swimps-log.h"

namespace swimps::option {
    //!
    //! \brief  Represents a configuration of swimps.
    //!
    struct Options {
        bool help = false;
        swimps::log::LogLevel logLevel = swimps::log::LogLevel::Info;
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

    //!
    //! \brief  An exception type emitted by parse_command_line
    //!         when parsing fails (e.g. when given invalid flags).
    //!
    class ParseException : public std::runtime_error {
    public:
        //!
        //! \brief  Constructs for ParseException.
        //!
        //! \param[in]  details  Whatever went wrong.
        //!
        ParseException(std::string details)
        : std::runtime_error(details) { }

        virtual ~ParseException() = default;

    private:
        std::string m_details;
    };

    //!
    //! \brief  An exception type emitted by parse_command_line
    //!         when an invalid value is provided for an option.
    //!
    class InvalidOptionValueException : public ParseException {
    public:
        //!
        //! \brief  Constructor for InvalidOptionValueException.
        //!
        //! \param[in]  option        Whichever option got the bad value.
        //! \param[in]  invalidValue  The bad value it got.
        //! \param[in]  validValues   Whatever is considered a value value.
        //!
        InvalidOptionValueException(
            std::string option,
            std::string invalidValue,
            std::string validValues
        ) : ParseException(
                option
              + " got invalid value "
              + invalidValue
              + " (valid values: "
              + validValues
              + ")"
        ) {

        }

        virtual ~InvalidOptionValueException() = default;
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
        const int argc,
        const char* argv[]);

    //!
    //! \brief  Prints the swimps help information to stdout.
    //!
    //! \note  This function is *not* async signal safe.
    //!
    void print_help();
}
