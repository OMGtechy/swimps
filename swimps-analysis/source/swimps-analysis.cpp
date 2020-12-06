#include "swimps-analysis.h"

#include <functional>

swimps::analysis::Analysis swimps::analysis::analyse(const swimps::trace::Trace& trace) {
    Analysis analysis;

    for (const auto& sample : trace.samples) {
        auto existingIter = std::find_if(
            analysis.backtraceFrequency.begin(),
            analysis.backtraceFrequency.end(),
            [sample](auto entry) {
                return entry.second == sample.backtraceID;
            }
        );

        if (existingIter != analysis.backtraceFrequency.end()) {
            existingIter->first += 1;
        } else {
            analysis.backtraceFrequency.emplace_back(1, sample.backtraceID);
        }
    }

    std::sort(
        analysis.backtraceFrequency.begin(),
        analysis.backtraceFrequency.end(),
        std::greater<>{}
    );

    return analysis;
}

