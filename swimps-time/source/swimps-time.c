#include "swimps-time.h"

int swimps_gettime(const clockid_t clockID, swimps_timespec_t* const out) {
    struct timespec timespecNow;
    const int returnValue = clock_gettime(clockID, &timespecNow);

    out->seconds = timespecNow.tv_sec;
    out->nanoseconds = timespecNow.tv_nsec;

    return returnValue;
}
