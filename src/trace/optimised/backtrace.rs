use std::hash::Hash;

use crate::trace::optimised::stack_frame::StackFrameID;

#[derive(Debug)]
pub struct BacktraceID(pub u64);

#[derive(Debug)]
pub struct Backtrace {
    pub id: BacktraceID,
    pub stack_frames: Vec<StackFrameID>
}

impl PartialEq for Backtrace {
    fn eq(&self, other: &Self) -> bool {
        self.id.0 == other.id.0
    }
}

impl Eq for Backtrace {}

impl Hash for Backtrace {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.id.0.hash(state)
    }
}
