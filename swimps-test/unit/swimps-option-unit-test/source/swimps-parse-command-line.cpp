#include "swimps-unit-test.h"
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

    ~MockArguments() {
        for (auto* arg : m_arguments) {
            free(arg);
        }
    }

    constexpr int argc() const noexcept { return N; } 
    constexpr const char** argv() const noexcept {
        return const_cast<const char**>(m_arguments.data());
    }

private:
    std::array<char*, N + 1> m_arguments = { };
};

using namespace swimps;

SCENARIO("swimps::option::parse_command_line", "[swimps-option]") {
    GIVEN("A debug log level option.") {
        MockArguments<4> args({
            "/fake/path/swimps",
            "--log-level",
            "debug",
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
        MockArguments<4> args({
            "/fake/path/swimps",
            "--log-level",
            "info",
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
        MockArguments<4> args({
            "/fake/path/swimps",
            "--log-level",
            "warning",
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
        MockArguments<4> args({
            "/fake/path/swimps",
            "--log-level",
            "error",
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
        MockArguments<4> args({
            "/fake/path/swimps",
            "--log-level",
            "fatal",
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
        MockArguments<2> args({
            "/fake/path/swimps",
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
