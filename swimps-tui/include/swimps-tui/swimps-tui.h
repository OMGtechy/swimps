#pragma once

#include "swimps-analysis/swimps-analysis.h"
#include "swimps-error/swimps-error.h"
#include "swimps-trace/swimps-trace.h"

namespace swimps::tui {
    swimps::error::ErrorCode run(const swimps::trace::Trace&, const swimps::analysis::Analysis&);
}
