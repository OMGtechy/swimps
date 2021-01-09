#include "swimps-analysis.h"

#include <functional>

using swimps::analysis::Analysis;
using swimps::trace::stack_frame_count_t;
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

    std::vector<Analysis::CallTreeNode> get_call_tree(const Trace& trace) {
        Analysis::CallTreeNode root({0, 0, {}});

        for(const auto& backtrace : trace.backtraces) {
            auto* targetNodeChildren = &root.children;

            for(stack_frame_count_t i = backtrace.stackFrameIDCount; i > 0; --i) {
                const auto stackFrameID = backtrace.stackFrameIDs[i - 1];

                const auto existingChild = std::find_if(
                    targetNodeChildren->begin(),
                    targetNodeChildren->end(),
                    [stackFrameID](const auto& child){
                        return child.stackFrameID == stackFrameID;
                    }
                );

                if (existingChild != targetNodeChildren->end()) {
                    existingChild->frequency += 1;
                    targetNodeChildren = &existingChild->children;
                } else {
                    targetNodeChildren->push_back({1, stackFrameID, {}});
                    targetNodeChildren = &targetNodeChildren->back().children;
                }
            }
        }

        return root.children;
    }
}

Analysis swimps::analysis::analyse(const Trace& trace) {
    Analysis analysis;

    analysis.backtraceFrequency = get_backtrace_frequency(trace);
    analysis.callTree = get_call_tree(trace);

    return analysis;
}

