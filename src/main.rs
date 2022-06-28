pub mod args;
pub mod child;
pub mod parent;
pub mod tui;

use args::Args;
use child::child;
use fork::{fork, Fork};
use parent::parent;
use nix::unistd::Pid;

fn main() {
    let args = Args::parse();
    match fork() {
        Ok(Fork::Parent(child_pid)) => { parent(Pid::from_raw(child_pid)) },
        Ok(Fork::Child) => { child(&args) },
        Err(error) => { println!("Error {} on fork.", error) }
    }

    if args.tui() {
        tui::run(args);
    }
}
