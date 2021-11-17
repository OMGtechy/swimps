#include "swimps-unit-test.h"
#include "swimps-option/swimps-option-parser.h"

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
            const auto maybeOptions = option::parse_command_line(
                args.argc(),
                args.argv()
            );

            THEN("parsing succeeds.") {
                REQUIRE(maybeOptions.has_value());

                AND_THEN("The log level is set accordingly.") {
                    REQUIRE(maybeOptions->logLevel == log::LogLevel::Debug);
                }
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
            const auto maybeOptions = option::parse_command_line(
                args.argc(),
                args.argv()
            );

            THEN("parsing succeeds.") {
                REQUIRE(maybeOptions.has_value());

                AND_THEN("The log level is set accordingly.") {
                    REQUIRE(maybeOptions->logLevel == log::LogLevel::Info);
                }
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
            const auto maybeOptions = option::parse_command_line(
                args.argc(),
                args.argv()
            );

            THEN("parsing succeeds.") {
                REQUIRE(maybeOptions.has_value());

                AND_THEN("The log level is set accordingly.") {
                    REQUIRE(maybeOptions->logLevel == log::LogLevel::Warning);
                }
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
            const auto maybeOptions = option::parse_command_line(
                args.argc(),
                args.argv()
            );

            THEN("parsing succeeds.") {
                REQUIRE(maybeOptions.has_value());

                AND_THEN("The log level is set accordingly.") {
                    REQUIRE(maybeOptions->logLevel == log::LogLevel::Error);
                }
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
            const auto maybeOptions = option::parse_command_line(
                args.argc(),
                args.argv()
            );

            THEN("parsing succeeds.") {
                REQUIRE(maybeOptions.has_value());

                AND_THEN("The log level is set accordingly.") {
                    REQUIRE(maybeOptions->logLevel == log::LogLevel::Fatal);
                }
            }
        }
    }

    GIVEN("A target program option.") {
        MockArguments<2> args({
            "/fake/path/swimps",
            "stress",
        });

        WHEN("They are parsed.") {
            const auto maybeOptions = option::parse_command_line(
                args.argc(),
                args.argv()
            );

            THEN("parsing succeeds.") {
                REQUIRE(maybeOptions.has_value());

                AND_THEN("The target program is set accordingly.") {
                    REQUIRE(maybeOptions->targetProgram == "stress");
                }
            }
        }
    }

    GIVEN("A target trace file option.") {
        MockArguments<4> args({
            "/fake/path/swimps",
            "--target-trace-file",
            "myfile.txt",
            "dummy"
        });

        WHEN("They are parsed.") {
            const auto maybeOptions = option::parse_command_line(
                args.argc(),
                args.argv()
            );

            THEN("parsing succeeds.") {
                REQUIRE(maybeOptions.has_value());

                AND_THEN("The target trace file is set accordingly.") {
                    REQUIRE(maybeOptions->targetTraceFile == "myfile.txt");
                }
            }
        }

    }

    GIVEN("A no ptrace option.") {
        MockArguments<3> args({
            "/fake/path/swimps",
            "--no-ptrace",
            "dummy"
        });

        WHEN("It is parsed.") {
            const auto maybeOptions = option::parse_command_line(
                args.argc(),
                args.argv()
            );

            THEN("parsing succeeds.") {
                REQUIRE(maybeOptions.has_value());

                AND_THEN("The ptrace option is set accordingly.") {
                    REQUIRE(! maybeOptions->ptrace);
                }
            }
        }
    }

    GIVEN("An invalid option value." ) {
        MockArguments<3> args({
            "/fake/path/swimps",
            "--log-level",
            "monty python"
        });

        WHEN("The options are parsed.") {
            const auto maybeOptions = option::parse_command_line(args.argc(), args.argv());

            THEN("parsing fails.") {
                REQUIRE(! maybeOptions.has_value());
            }
        }
    }
}
