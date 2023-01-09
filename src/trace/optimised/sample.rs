use crate::trace::optimised::backtrace::BacktraceID;
use crate::trace::shared::timestamp::Timestamp;

#[derive(Debug)]
pub struct Sample {
    pub backtrace: BacktraceID,
    pub timestamp: Timestamp
}
