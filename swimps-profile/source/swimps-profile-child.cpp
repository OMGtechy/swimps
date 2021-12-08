#include "swimps-profile/swimps-profile.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <vector>

#include <unistd.h>
#include <sys/ptrace.h>
#include <linux/limits.h>

#include <codeinjector/inject.hpp>

#include <signalsafe/memory.hpp>
#include <signalsafe/string.hpp>

#include "swimps-error/swimps-error.h"
#include "swimps-log/swimps-log.h"
#include "swimps-option/swimps-option-parser.h"

using codeinjector::inject_library;

using signalsafe::memory::copy_no_overlap;
using signalsafe::string::format;

using swimps::error::ErrorCode;

swimps::error::ErrorCode swimps::profile::child(const swimps::option::Options& options) {
    // LCOV_EXCL_START
    if (options.targetProgram.empty()) {
        swimps::log::write_to_log(
            swimps::log::LogLevel::Fatal,
            "No program specified."
        );

        return swimps::error::ErrorCode::InvalidParameter;
    }

    // Enable tracing of the program we're about to exec into
    if (options.ptrace) {
        swimps::log::write_to_log(
            swimps::log::LogLevel::Debug,
            "Running ptrace(PTRACE_TRACEME)."
        );

        if (ptrace(PTRACE_TRACEME) == -1) {
            swimps::log::format_and_write_to_log<128>(
                swimps::log::LogLevel::Fatal,
                "ptrace(PTRACE_TRACEME) failed, errno % (%).",
                errno,
                strerror(errno)
            );

            return swimps::error::ErrorCode::PtraceFailed;
        }
    }

    std::array<char, PATH_MAX> swimpsPathBuffer = { 0 };
    const auto swimpsPathBufferBytes = readlink(
        "/proc/self/exe",
        swimpsPathBuffer.data(),
        swimpsPathBuffer.size()
    );

    if (swimpsPathBufferBytes < 0) {
        return ErrorCode::ReadlinkFailed;
    }

    swimpsPathBuffer[std::max(swimpsPathBuffer.size() - 1, static_cast<std::size_t>(swimpsPathBufferBytes))] = '\0';

    auto swimpsPath = std::filesystem::path(swimpsPathBuffer.data());
    swimpsPath.remove_filename();

    auto preloadPath = swimpsPath;
    preloadPath.append("swimps-preload/libswimps-preload.so");

    std::vector<std::string_view> args(options.targetProgramArgs.cbegin(), options.targetProgramArgs.cend());

    setenv("SWIMPS_OPTIONS", options.toString().c_str(), true);
    inject_library(options.targetProgram, args, preloadPath);

    // We only get here if the something went wrong.
    {

        swimps::log::format_and_write_to_log<128>(
            swimps::log::LogLevel::Fatal,
            "Failed to execute target program, errno % (%).",
            errno,
            strerror(errno)
        );
    }

    return ErrorCode::InjectLibraryFailed;
    // LCOV_EXCL_STOP
}
