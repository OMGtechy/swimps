#pragma once

#include <ctime>

namespace swimps::time {
    //!
    //! \brief  A shorthand for creating a timer that fires a signal.
    //!
    //! \param[in]   clockID  The clock you want the timer to use.
    //! \param[in]   signal   The signal you want to fire when triggered.
    //! \param[out]  out      The resulting timer.
    //!
    //! \returns  0 on success, -1 on error (errno may be set accordingly).
    //!
    //! \note  It does not start the timer.
    //!
    //! \note  This function is *not* async signal safe.
    //!
    int create_signal_timer(const clockid_t clockID, const int signal, timer_t& out);
}
