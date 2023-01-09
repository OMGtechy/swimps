use std::{collections::HashSet, hash::Hash};

use crate::trace::optimised::{backtrace::{Backtrace, BacktraceID}, sample::Sample, stack_frame::{StackFrame, StackFrameID}};
use crate::trace::raw::{sample::Sample as RawSample, trace::Trace as RawTrace};
use crate::trace::shared::{instruction_pointer::InstructionPointer, timestamp::Timestamp};

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

struct BacktraceStackFramesEqualityWrapper(pub Backtrace);

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
pub struct Trace {
    pub stack_frames: HashSet::<StackFrame>,
    pub backtraces: HashSet::<Backtrace>,
    pub samples: Vec<Sample>
}

fn raw_to_optimised_sample(raw_sample: &RawSample,
                           stack_frames: &mut HashSet::<StackFrameInstructionPointerEqualityWrapper>,
                           next_stack_frame_id: &mut StackFrameID,
                           backtraces: &mut HashSet::<BacktraceStackFramesEqualityWrapper>,
                           next_backtrace_id: &mut BacktraceID) -> Sample {
    let new_backtrace = BacktraceStackFramesEqualityWrapper(
        Backtrace {
            id: BacktraceID(next_backtrace_id.0),
            stack_frames: raw_sample.backtrace.0.iter().map(|ip|
                get_or_insert_stack_frame(
                    StackFrameInstructionPointerEqualityWrapper(StackFrame {
                        id: StackFrameID(next_stack_frame_id.0),
                        instruction_pointer: InstructionPointer(ip.0)
                    }),
                    stack_frames,
                    next_stack_frame_id)
            ).collect()
        }
    );

    Sample {
        timestamp: Timestamp { seconds: raw_sample.timestamp.seconds, nanoseconds: raw_sample.timestamp.nanoseconds },
        backtrace: match backtraces.get(&new_backtrace) {
            None => {
                let id = new_backtrace.0.id.0;
                backtraces.insert(new_backtrace);
                *next_backtrace_id = BacktraceID(next_backtrace_id.0.checked_add(1).expect("Backtrace ID overflowed"));
                BacktraceID(id)
            }
            Some(bt) => BacktraceID(bt.0.id.0)
        }
    }
}

fn get_or_insert_stack_frame(to_insert: StackFrameInstructionPointerEqualityWrapper,
                             stack_frames: &mut HashSet::<StackFrameInstructionPointerEqualityWrapper>,
                             next_stack_frame_id: &mut StackFrameID) -> StackFrameID {
    match stack_frames.get(&to_insert) {
        None => {
            let id = to_insert.0.id.0;
            stack_frames.insert(to_insert);
            *next_stack_frame_id = StackFrameID(
                next_stack_frame_id.0.checked_add(1).expect("Stack frame ID overflowed")
            );
            StackFrameID(id)
        }
        Some(sf) => StackFrameID(sf.0.id.0)
    }
}

impl Trace {
    pub fn new(raw_trace: RawTrace) -> Trace {
        let mut stack_frames = HashSet::<StackFrameInstructionPointerEqualityWrapper>::new();
        let mut next_stack_frame_id = StackFrameID(1);

        let mut backtraces = HashSet::<BacktraceStackFramesEqualityWrapper>::new();
        let mut next_backtrace_id = BacktraceID(1);

        let samples = raw_trace.samples().iter().map(
            |raw_sample|
                raw_to_optimised_sample(
                    raw_sample,
                    &mut stack_frames,
                    &mut next_stack_frame_id,
                    &mut backtraces,
                    &mut next_backtrace_id)).collect();

        Trace {
            stack_frames: stack_frames.drain().map(|wrapper| wrapper.0).collect(),
            samples,
            backtraces: backtraces.drain().map(|wrapper| wrapper.0).collect()
        }
    }
}
