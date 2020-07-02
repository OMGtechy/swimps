#include "swimps-time.h"
#include "swimps-assert.h"

#include <signal.h>

int swimps_gettime(const clockid_t clockID, swimps_timespec_t* const out) {

    swimps_assert(out != NULL);

    struct timespec timespecNow;
    const int returnValue = clock_gettime(clockID, &timespecNow);

    out->seconds = timespecNow.tv_sec;
    out->nanoseconds = timespecNow.tv_nsec;

    return returnValue;
}

int swimps_create_signal_timer(const clockid_t clockID, const int signal, timer_t* const out) {

    swimps_assert(out != NULL);

    struct sigevent signalEvent;
    signalEvent.sigev_notify = SIGEV_SIGNAL;
    signalEvent.sigev_signo = signal;

    return timer_create(clockID, &signalEvent, out);
}
