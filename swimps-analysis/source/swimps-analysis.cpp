#include "swimps-analysis.h"

#include <functional>

using swimps::analysis::Analysis;
using swimps::trace::Trace;

namespace {
    Analysis::BacktraceFrequency get_backtrace_frequency(const Trace& trace) {
        Analysis::BacktraceFrequency backtraceFrequency;

        for (const auto& sample : trace.samples) {
            auto existingIter = std::find_if(
                backtraceFrequency.begin(),
                backtraceFrequency.end(),
                [sample](auto entry) {
                    return entry.second == sample.backtraceID;
                }
            );

            if (existingIter != backtraceFrequency.end()) {
                existingIter->first += 1;
            } else {
                backtraceFrequency.emplace_back(1, sample.backtraceID);
            }
        }

        std::sort(
            backtraceFrequency.begin(),
            backtraceFrequency.end(),
            std::greater<>{}
        );

        return backtraceFrequency;
    }
}

Analysis swimps::analysis::analyse(const Trace& trace) {
    Analysis analysis;

    analysis.backtraceFrequency = get_backtrace_frequency(trace);

    return analysis;
}

