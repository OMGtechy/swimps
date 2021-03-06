#pragma once

#include <vector>

#include "swimps-trace/swimps-trace.h"

namespace swimps::analysis {
    struct Analysis {
        using BacktraceFrequency = std::vector<
            std::pair<
                swimps::trace::sample_count_t,
                swimps::trace::backtrace_id_t
            >
        >;

        struct CallTreeNode {
            swimps::trace::sample_count_t frequency;
            swimps::trace::stack_frame_id_t stackFrameID;
            std::vector<CallTreeNode> children;
        };

        BacktraceFrequency backtraceFrequency;
        std::vector<CallTreeNode> callTree;
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

