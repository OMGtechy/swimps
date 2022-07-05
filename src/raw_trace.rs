use std::os::raw::{c_void, c_uchar};

extern "C" {
    fn samplerpreload_trace_from(data: *const c_uchar, data_length: usize) -> *const c_void;
    fn samplerpreload_trace_dtor(trace: *const c_void);
    fn samplerpreload_trace_get_sample_count(trace: *const c_void) -> usize;
    fn samplerpreload_trace_get_sample_reference(trace: *const c_void, n: usize) -> *const c_void;

    fn samplerpreload_sample_get_timestamp_seconds(sample: *const c_void) -> i64;
    fn samplerpreload_sample_get_timestamp_nanoseconds(sample: *const c_void) -> i64;
    fn samplerpreload_sample_get_backtrace_size(sample: *const c_void) -> usize;
    fn samplerpreload_sample_get_backtrace_entry(sample: *const c_void, n: usize) -> u64;
}

#[derive(Debug, PartialEq, Eq)]
pub struct InstructionPointer(pub u64);

#[derive(Debug)]
pub struct Backtrace(pub Vec<InstructionPointer>);

#[derive(Debug)]
pub struct Timestamp {
    pub seconds: i64,
    pub nanoseconds: i64
}

#[derive(Debug)]
pub struct Sample {
    pub backtrace: Backtrace,
    pub timestamp: Timestamp
}

#[derive(Debug)]
pub struct RawTrace {
    pub samples: Vec<Sample>
}

impl RawTrace {
    pub fn from(data: Vec<u8>) -> RawTrace {
        let c_trace = unsafe { samplerpreload_trace_from(data.as_ptr(), data.len()) };
        let c_sample_count = unsafe { samplerpreload_trace_get_sample_count(c_trace) };

        let c_samples =
            (0..c_sample_count)
                .map(
                    |n| unsafe { samplerpreload_trace_get_sample_reference(c_trace, n) });

        let get_timestamp = |c_sample| {
            Timestamp {
                seconds: unsafe { samplerpreload_sample_get_timestamp_seconds(c_sample) },
                nanoseconds: unsafe { samplerpreload_sample_get_timestamp_nanoseconds(c_sample) }
            }
        };

        let get_backtrace_size = |c_sample| {
            unsafe { samplerpreload_sample_get_backtrace_size(c_sample) }
        };

        let get_backtrace_entry = |c_sample, n| {
            InstructionPointer(unsafe { samplerpreload_sample_get_backtrace_entry(c_sample, n) })
        };

        let get_backtrace = |c_sample| {
            Backtrace (
                (0..get_backtrace_size(c_sample))
                    .map(|n| get_backtrace_entry(c_sample, n) )
                    .collect()
            )
        };

        let result = RawTrace {
            samples:
                c_samples.map(
                    |c_sample| Sample {
                        backtrace: get_backtrace(c_sample),
                        timestamp: get_timestamp(c_sample)
                    }
                ).collect()
        };

        unsafe { samplerpreload_trace_dtor(c_trace) };

        result
    }

    pub fn samples(&self) -> &Vec<Sample> {
        &self.samples
    }
}
