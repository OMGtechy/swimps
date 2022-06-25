pub mod args;
pub mod child;

use args::Args;
use child::child;
use fork::{fork, Fork};

fn main() {
    let args = Args::parse();
    match fork() {
        Ok(Fork::Parent(child_pid)) => { println!("Child process {} started.", child_pid)},
        Ok(Fork::Child) => { child(&args) },
        Err(error) => { println!("Error {} on fork.", error) }
    } 
}
