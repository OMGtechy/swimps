#include "swimps-time.h"
#include "swimps-assert.h"

int swimps_gettime(const clockid_t clockID, swimps_timespec_t* const out) {

    swimps_assert(out != NULL);

    struct timespec timespecNow;
    const int returnValue = clock_gettime(clockID, &timespecNow);

    out->seconds = timespecNow.tv_sec;
    out->nanoseconds = timespecNow.tv_nsec;

    return returnValue;
}
