use std::collections::HashMap;

use crate::trace::optimised::{trace::Trace, backtrace::BacktraceID};

pub fn backtrace_by_frequency(optimised_trace: &Trace) -> HashMap::<BacktraceID, usize> {
    let mut frequencies = HashMap::<BacktraceID, usize>::new();

    optimised_trace
        .samples
        .iter()
        .for_each(|sample| {
            *frequencies
            .entry(BacktraceID(sample.backtrace.0))
            .or_insert(0) += 1; });

    frequencies
}