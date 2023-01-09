use crate::trace::raw::backtrace::Backtrace;
use crate::trace::shared::timestamp::Timestamp;

#[derive(Debug)]
pub struct Sample {
    pub backtrace: Backtrace,
    pub timestamp: Timestamp
}
