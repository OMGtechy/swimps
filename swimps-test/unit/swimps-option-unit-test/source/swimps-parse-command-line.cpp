#include "swimps-test.h"
#include "swimps-option.h"

#include <array>
#include <cstring>

template <size_t N>
class MockArguments final {
public:
    MockArguments(std::array<const char*, N> arguments) {
        for (size_t i = 0; i < arguments.size(); ++i) {
            m_arguments[i] = strdup(arguments[i]);
        }
    };

    constexpr int argc() const noexcept { return N; } 
    constexpr char** argv() noexcept {
        return m_arguments.data();
    }

private:
    // TODO: Should these be free'd?
    //       Got a double free when I tried to...
    //       Example use cases make it look like I should though.
    //       A potential leak in the tests isn't so bad in the meantime.
    std::array<char*, N + 1> m_arguments = { };
};

using namespace swimps;

SCENARIO("swimps::option::parse_command_line", "[swimps-option]") {
    GIVEN("A debug log level option.") {
        MockArguments<5> args({
            "/fake/path/swimps",
            "--log-level",
            "debug",
            "--target-program",
            "dummy"
        });

        WHEN("They are parsed.") {
            const auto options = option::parse_command_line(
                args.argc(),
                args.argv()
            );

            THEN("The log level is set accordingly.") {
                REQUIRE(options.logLevel == log::LogLevel::Debug);
            }
        }
    }

    GIVEN("An info log level option.") {
        MockArguments<5> args({
            "/fake/path/swimps",
            "--log-level",
            "info",
            "--target-program",
            "dummy"
        });

        WHEN("They are parsed.") {
            const auto options = option::parse_command_line(
                args.argc(),
                args.argv()
            );

            THEN("The log level is set accordingly.") {
                REQUIRE(options.logLevel == log::LogLevel::Info);
            }
        }
    }

    GIVEN("A warning log level option.") {
        MockArguments<5> args({
            "/fake/path/swimps",
            "--log-level",
            "warning",
            "--target-program",
            "dummy"
        });

        WHEN("They are parsed.") {
            const auto options = option::parse_command_line(
                args.argc(),
                args.argv()
            );

            THEN("The log level is set accordingly.") {
                REQUIRE(options.logLevel == log::LogLevel::Warning);
            }
        }
    }

    GIVEN("An error log level option.") {
        MockArguments<5> args({
            "/fake/path/swimps",
            "--log-level",
            "error",
            "--target-program",
            "dummy"
        });

        WHEN("They are parsed.") {
            const auto options = option::parse_command_line(
                args.argc(),
                args.argv()
            );

            THEN("The log level is set accordingly.") {
                REQUIRE(options.logLevel == log::LogLevel::Error);
            }
        }
    }

    GIVEN("A fatal log level option.") {
        MockArguments<5> args({
            "/fake/path/swimps",
            "--log-level",
            "fatal",
            "--target-program",
            "dummy"
        });

        WHEN("They are parsed.") {
            const auto options = option::parse_command_line(
                args.argc(),
                args.argv()
            );

            THEN("The log level is set accordingly.") {
                REQUIRE(options.logLevel == log::LogLevel::Fatal);
            }
        }
    
}
    GIVEN("A target program option.") {
        MockArguments<3> args({
            "/fake/path/swimps",
            "--target-program",
            "stress",
        });

        WHEN("They are parsed.") {
            const auto options = option::parse_command_line(
                args.argc(),
                args.argv()
            );

            THEN("The log level is set accordingly.") {
                REQUIRE(options.targetProgram == "stress");
            }
        }
    }
}
