#pragma once

#include "swimps-analysis.h"
#include "swimps-error.h"
#include "swimps-trace.h"

namespace swimps::tui {
    swimps::error::ErrorCode run(const swimps::trace::Trace&, const swimps::analysis::Analysis&);
}
