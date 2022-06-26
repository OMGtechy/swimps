use nix::sys::wait::waitpid;
use nix::unistd::Pid;

pub fn parent(child_pid: Pid) {
    match waitpid(child_pid, None) {
        Ok(_) => (),
        Err(errno) => println!("waitpid returned error {}", errno)
    }
}