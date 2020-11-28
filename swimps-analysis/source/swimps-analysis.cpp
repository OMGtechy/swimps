#include "swimps-analysis.h"

swimps::analysis::Analysis swimps::analysis::analyse(const swimps::trace::Trace& trace) {
    Analysis analysis;

    for (const auto& sample : trace.samples) {
        analysis.backtraceFrequency[sample.backtraceID] += 1;
    }

    return analysis;
}

