pub mod args;
pub mod child;
pub mod parent;

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
}
