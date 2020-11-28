#pragma once

#include <map>

#include "swimps-trace.h"

namespace swimps::analysis {
    struct Analysis {
        using BacktraceFrequency = std::map<
            swimps::trace::backtrace_id_t,
            swimps::trace::sample_count_t
        >;

        BacktraceFrequency backtraceFrequency;
    };

    //!
    //! \brief  Performs analysis upon a trace.
    //!
    //! \param[in]  trace  The trace to analyse.
    //!
    //! \returns  The analysis results.
    //!
    //! \note  This function is *not* async signal safe.
    //!
    Analysis analyse(const swimps::trace::Trace& trace);
}

