use std::{collections::HashSet, hash::Hash};

use crate::trace::raw_trace::{RawTrace, InstructionPointer, Timestamp};

#[derive(Debug)]
pub struct StackFrameID(u64);

#[derive(Debug)]
pub struct StackFrame {
    pub id: StackFrameID,
    pub instruction_pointer: InstructionPointer,
    // TODO: add function name, line number...
}

impl PartialEq for StackFrame {
    fn eq(&self, other: &Self) -> bool {
        self.id.0 == other.id.0
    }
}

impl Eq for StackFrame {}

impl Hash for StackFrame {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.id.0.hash(state)
    }
}

struct StackFrameInstructionPointerEqualityWrapper(pub StackFrame);

impl PartialEq for StackFrameInstructionPointerEqualityWrapper {
    fn eq(&self, other: &Self) -> bool {
        self.0.instruction_pointer == other.0.instruction_pointer
    }
}

impl Eq for StackFrameInstructionPointerEqualityWrapper {}

impl Hash for StackFrameInstructionPointerEqualityWrapper {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.0.instruction_pointer.0.hash(state)
    }
}

#[derive(Debug)]
pub struct BacktraceID(u64);

#[derive(Debug)]
pub struct OptimisedBacktrace {
    id: BacktraceID,
    stack_frames: Vec<StackFrameID>
}

impl PartialEq for OptimisedBacktrace {
    fn eq(&self, other: &Self) -> bool {
        self.id.0 == other.id.0
    }
}

impl Eq for OptimisedBacktrace {}

impl Hash for OptimisedBacktrace {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.id.0.hash(state)
    }
}

struct BacktraceStackFramesEqualityWrapper(pub OptimisedBacktrace);

impl PartialEq for BacktraceStackFramesEqualityWrapper {
    fn eq(&self, other: &Self) -> bool {
        self.0.stack_frames.iter().map(|sf| sf.0).collect::<Vec<_>>() == other.0.stack_frames.iter().map(|sf| sf.0).collect::<Vec<_>>()
    }
}

impl Eq for BacktraceStackFramesEqualityWrapper {}

impl Hash for BacktraceStackFramesEqualityWrapper {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.0.stack_frames.iter().for_each(|sf| { sf.0.hash(state); })
    }
}

#[derive(Debug)]
pub struct OptimisedSample {
    backtrace: BacktraceID,
    timestamp: Timestamp
}

#[derive(Debug)]
pub struct OptimisedTrace {
    pub stack_frames: HashSet::<StackFrame>,
    pub backtraces: HashSet::<OptimisedBacktrace>,
    pub samples: Vec<OptimisedSample>    
}

impl OptimisedTrace {
    pub fn new(raw_trace: RawTrace) -> OptimisedTrace {
        let mut stack_frames = HashSet::<StackFrameInstructionPointerEqualityWrapper>::new();
        let mut next_stack_frame_id = StackFrameID(1);

        let mut backtraces = HashSet::<BacktraceStackFramesEqualityWrapper>::new();
        let mut next_backtrace_id = BacktraceID(1);

        let mut samples = Vec::<OptimisedSample>::new();

        for raw_sample in raw_trace.samples() {
            let mut new_backtrace = BacktraceStackFramesEqualityWrapper(
                OptimisedBacktrace { id: BacktraceID(next_backtrace_id.0), stack_frames: vec![] }
            );

            raw_sample.backtrace.0.iter().for_each(|ip| {
                let new_stack_frame = StackFrameInstructionPointerEqualityWrapper(StackFrame {
                    id: StackFrameID(next_stack_frame_id.0),
                    instruction_pointer: InstructionPointer(ip.0)
                });

                new_backtrace.0.stack_frames.push(match stack_frames.get(&new_stack_frame) {
                    None => {
                        let id = new_stack_frame.0.id.0;
                        stack_frames.insert(new_stack_frame);
                        next_stack_frame_id = StackFrameID(next_stack_frame_id.0.checked_add(1).expect("Stack frame ID overflowed"));
                        StackFrameID(id)
                    }
                    Some(sf) => StackFrameID(sf.0.id.0)
                });
            });

            samples.push(OptimisedSample {
                timestamp: Timestamp { seconds: raw_sample.timestamp.seconds, nanoseconds: raw_sample.timestamp.nanoseconds },
                backtrace: match backtraces.get(&new_backtrace) {
                    None => {
                        let id = new_backtrace.0.id.0;
                        backtraces.insert(new_backtrace);
                        next_backtrace_id = BacktraceID(next_backtrace_id.0.checked_add(1).expect("Backtrace ID overflowed"));
                        BacktraceID(id)
                    }
                    Some(bt) => BacktraceID(bt.0.id.0)
                }
            });
        }

        OptimisedTrace {
            stack_frames: stack_frames.drain().map(|wrapper| wrapper.0).collect(),
            samples,
            backtraces: backtraces.drain().map(|wrapper| wrapper.0).collect()
        }
    }
}
