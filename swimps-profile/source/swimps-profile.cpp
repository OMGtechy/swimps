#include "swimps-profile.h"
#include "swimps-log.h"
#include "swimps-io.h"

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

swimps::error::ErrorCode swimps_profile(char** args) {
    const pid_t pid = fork();

    switch(pid) {
    case -1: {
        // Fork failed.
        swimps::log::format_and_write_to_log<128>(
            swimps::log::LogLevel::Fatal,
            "fork failed, errno %d (%s).",
            errno,
            strerror(errno)
        );

        return swimps::error::ErrorCode::ForkFailed;
    }
    case 0:
        return swimps_profile_child(args);
    default:
        return swimps_profile_parent(pid);
    }
}

