pub mod args;
pub mod child;
pub mod optimised_trace;
pub mod parent;
pub mod raw_trace;
pub mod tui;

use args::Args;
use child::child;
use fork::{fork, Fork};
use parent::parent;
use nix::unistd::Pid;

fn main() {
    let args = Args::parse();

    // Force trace file evaluation so that parent and child don't get different data.
    // This is code smell...
    args.trace_file();

    match fork() {
        Ok(Fork::Parent(child_pid)) => { parent(args, Pid::from_raw(child_pid)) },
        Ok(Fork::Child) => { child(&args) },
        Err(error) => { println!("Error {} on fork.", error) }
    }
}
