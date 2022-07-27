use std::hash::Hash;

use crate::trace::shared::instruction_pointer::InstructionPointer;

#[derive(Debug, PartialEq, Eq, Hash)]
pub struct StackFrameID(pub u64);

#[derive(Debug)]
pub struct StackFrame {
    pub id: StackFrameID,
    pub instruction_pointer: InstructionPointer,
}

impl PartialEq for StackFrame {
    fn eq(&self, other: &Self) -> bool {
        self.id == other.id
    }
}

impl Eq for StackFrame {}

impl Hash for StackFrame {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.id.hash(state)
    }
}
