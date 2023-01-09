use crate::args::Args;
use crate::tui;

use nix::sys::wait::waitpid;
use nix::unistd::Pid;

pub fn parent(args: Args, child_pid: Pid) {
    match waitpid(child_pid, None) {
        Ok(_) => (),
        Err(errno) => println!("waitpid returned error {}", errno)
    }

    if args.tui() {
        tui::run(args);
    }
}