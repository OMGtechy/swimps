use crate::trace::shared::instruction_pointer::InstructionPointer;

#[derive(Debug)]
pub struct Backtrace(pub Vec<InstructionPointer>);
