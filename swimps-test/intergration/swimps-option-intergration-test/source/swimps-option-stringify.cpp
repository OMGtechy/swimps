#include "swimps-intergration-test.h"
#include "swimps-option/swimps-option-options.h"

SCENARIO("swimps::option::Options::toString, swimps::option::Options::fromString", "[swimps-option]") {

    using swimps::option::Options;

    GIVEN("Default options.") {
        const Options original;

        WHEN("They are converted to a string and back again.") {
            const auto converted = Options::fromString(original.toString());

            THEN("They are equivalent to the original.") {
                REQUIRE(original == converted);
                REQUIRE(! (original != converted));
            }
        }
    }

    GIVEN("Non-default options.") {
        const Options original {
            true,
            false,
            false,
            false,
            swimps::log::LogLevel::Debug,
            42,
            "amazing-swimps-trace-name",
            "programName",
            { "arg1", "arg2", "arg3" }
        };

        WHEN("They are converted to a string and back again.") {
            const auto converted = Options::fromString(original.toString());

            THEN("They are equivalent to the original.") {
                REQUIRE(original == converted);
                REQUIRE(! (original != converted));
            }
        }
    }
}
