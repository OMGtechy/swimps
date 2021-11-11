#include "swimps-time/swimps-time.h"

#include <signal.h>

int swimps::time::create_signal_timer(const clockid_t clockID, const int signal, timer_t& out) {

    sigevent signalEvent;
    signalEvent.sigev_notify = SIGEV_SIGNAL;
    signalEvent.sigev_signo = signal;

    return timer_create(clockID, &signalEvent, &out);
}
