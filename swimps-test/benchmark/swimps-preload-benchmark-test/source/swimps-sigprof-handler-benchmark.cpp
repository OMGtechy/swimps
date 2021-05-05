#include "swimps-benchmark-test.h"
#include "swimps-preload/private/sigprof_handler.h"

#define UNW_LOCAL_ONLY
#include <libunwind.h>

static void BM_sigprof_handler(benchmark::State& state) {
    unw_context_t context;
    #ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wgnu-statement-expression"
    #endif
    unw_getcontext(&context);
    #ifdef __clang__
    #pragma clang diagnostic pop
    #endif

    for(auto _ : state) {
        swimps::preload::sigprof_handler(0, nullptr, &context);
    }
}

BENCHMARK(BM_sigprof_handler);
