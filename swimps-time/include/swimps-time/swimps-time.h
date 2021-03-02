#pragma once

#include <ctime>
#include <cstdint>
#include <type_traits>

namespace swimps::time {
    // This is needed because swimps assumes that time_t and long are aliases for int64_t.
    // Things using swimps::time::TimeSpecification may break if that's not the case.
    static_assert(std::is_same_v<int64_t, long>);

    // This type exists because the standard timespec might not be the same across platforms,
    // meaning that one would only be able to read a profile on the platform it was built for.
    //
    // Having this means that we know exactly how big the timespec is and can read it everywhere.
    struct TimeSpecification {
        int64_t seconds;
        int64_t nanoseconds;
    };

    //!
    //! \brief  A more cross-platform version of clock_gettime.
    //!
    //! \param[in]   clockID  The kind of clock you want to get the time from.
    //! \param[out]  out      Where to write the resulting time.
    //!
    //! \returns  0 if all went well, -1 otherwise. Check errno for failure details.
    //!
    //! \note  This function is async signal safe.
    //!
    int now(const clockid_t clockID, swimps::time::TimeSpecification& out);

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
