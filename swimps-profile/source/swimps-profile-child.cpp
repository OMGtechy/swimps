#include "swimps-profile/swimps-profile.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <vector>

#include <unistd.h>
#include <sys/ptrace.h>
#include <linux/limits.h>

#include <signalsafe/memory.hpp>
#include <signalsafe/string.hpp>

#include "swimps-log/swimps-log.h"
#include "swimps-option/swimps-option-parser.h"

using signalsafe::memory::copy_no_overlap;
using signalsafe::string::format;

extern char** environ;

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

    // Work out how big the environment is
    // and where the existing LD_PRELOAD is (if there is one)
    const char ldPreloadStr[] = "LD_PRELOAD=";
    const char* existingLDPreload = NULL;
    size_t existingLDPreloadIndex = 0;

    size_t environmentSize = 0;
    while(environ[environmentSize] != NULL) {
        if (strncmp(environ[environmentSize], ldPreloadStr, sizeof ldPreloadStr) == 0) {
            existingLDPreload = environ[environmentSize];
            existingLDPreloadIndex = environmentSize;
        }

        environmentSize += 1;
    }

    // Add one for LD_PRELOAD if necessary
    if (existingLDPreload == NULL) {
        environmentSize += 1;
    }

    // Add one for SWIMPS_OPTIONS
    environmentSize += 1;

    // Add one for the NULL at the end
    environmentSize += 1;

    // Allocate space for the environment
    char** environment = static_cast<char**>(malloc(environmentSize * sizeof(char*)));

    // Copy over the existing environment (except LD_PRELOAD)
    size_t i = 0;
    for(; environ[i] != NULL; ++i) {
        if (environ[i] == existingLDPreload) {
            continue;
        }

        environment[i] = environ[i];
    }

    // Add LD_PRELOAD
    char absolutePathToLDPreload[PATH_MAX] = { 0 };

    if (existingLDPreload != NULL) {
        strcat(absolutePathToLDPreload, existingLDPreload);
        const char* separator = ":";
        strcat(absolutePathToLDPreload, separator);
    } else {
        strcat(absolutePathToLDPreload, ldPreloadStr);
    }

    const ssize_t swimpsPathBufferBytes = readlink(
        "/proc/self/exe",
        absolutePathToLDPreload + strnlen(absolutePathToLDPreload, sizeof absolutePathToLDPreload),
        (sizeof absolutePathToLDPreload) - strnlen(absolutePathToLDPreload, sizeof absolutePathToLDPreload)
    );

    if (swimpsPathBufferBytes == 0) {

        swimps::log::format_and_write_to_log<128>(
            swimps::log::LogLevel::Fatal,
            "readlink failed, errno % (%).",
            errno,
            strerror(errno)
        );

        return swimps::error::ErrorCode::ReadlinkFailed;
    }

    const char* preloadLibPathSuffix = "-preload/libswimps-preload.so";
    strcat(absolutePathToLDPreload, preloadLibPathSuffix);

    if (existingLDPreload != NULL) {
        environment[existingLDPreloadIndex] = absolutePathToLDPreload;
    } else {
        environment[i++] = absolutePathToLDPreload;
    }

    // Add SWIMPS_OPTIONS
    environment[i++] = strdup((std::string("SWIMPS_OPTIONS=") + options.toString()).c_str());

    environment[i] = NULL;

    {
        char targetBuffer[2048] = { };
        std::span<char> targetSpan(targetBuffer);

        targetSpan = targetSpan.last(targetSpan.size() - copy_no_overlap(
            std::span<const char>{ options.targetProgram.c_str(), options.targetProgram.length() },
            targetSpan
        ));

        for (auto& arg : options.targetProgramArgs) {
            targetSpan = targetSpan.last(targetSpan.size() - format(
                " %",
                targetSpan,
                arg.c_str()
            ));
        }

        swimps::log::write_to_log(
            swimps::log::LogLevel::Info,
            { targetBuffer, sizeof(targetBuffer) - targetSpan.size() }
        );
    }

    for(char** envIter = environment; *envIter != NULL; ++envIter) {
        swimps::log::write_to_log(
            swimps::log::LogLevel::Debug,
            { *envIter, strlen(*envIter) }
        );
    }

    std::vector<char*> argv;
    argv.push_back(strdup(options.targetProgram.c_str()));

    for (auto& arg : options.targetProgramArgs) {
        argv.push_back(strdup(arg.c_str()));
    }

    argv.push_back(nullptr);

    execvpe(argv[0], &argv[0], environment);

    // we only get here if the exec failed
    {

        swimps::log::format_and_write_to_log<128>(
            swimps::log::LogLevel::Fatal,
            "Failed to execute target program, errno % (%).",
            errno,
            strerror(errno)
        );
    }

    return swimps::error::ErrorCode::ExecveFailed;
    // LCOV_EXCL_STOP
}
