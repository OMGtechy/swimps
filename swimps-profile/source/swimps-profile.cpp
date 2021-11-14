#include "swimps-profile/swimps-profile.h"
#include "swimps-log/swimps-log.h"

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

swimps::error::ErrorCode swimps::profile::start(const swimps::option::Options& options) {
    const pid_t pid = fork();

    switch(pid) {
    case -1: {
        // Fork failed.
        swimps::log::format_and_write_to_log<128>(
            swimps::log::LogLevel::Fatal,
            "fork failed, errno % (%).",
            errno,
            strerror(errno)
        );

        return swimps::error::ErrorCode::ForkFailed;
    }
    case 0:
        return swimps::profile::child(options);
    default:
        return swimps::profile::parent(pid);
    }
}

