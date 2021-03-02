#include "swimps-benchmark-test.h"
#include "swimps-preload/private/sigprof_handler.h"

#include <libunwind.h>

static void BM_sigprof_handler(benchmark::State& state) {
    unw_context_t context;
    unw_getcontext(&context);

    for(auto _ : state) {
        swimps::preload::sigprof_handler(0, nullptr, &context);
    }
}

BENCHMARK(BM_sigprof_handler);
