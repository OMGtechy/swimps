pub mod args;

use std::ffi::CString;
use std::os::raw::c_char;

extern "C" {
    fn codeinjector_inject_library(program: *const c_char, args: *const *const c_char, argsCount: usize, lib: *const c_char) -> bool;
}

fn main() {
    let args = args::Args::parse();
    let dummy_args = [
        CString::new("a").unwrap(),
        CString::new("b").unwrap(),
        CString::new("c").unwrap()
    ];

    let dummy_arg_ptrs = dummy_args.map(|arg| arg.as_ptr());
    let dummy_program = CString::new("testprog").unwrap();
    let dummy_lib = CString::new("testlib").unwrap();

    unsafe {
        codeinjector_inject_library(
            dummy_program.as_ptr(),
            dummy_arg_ptrs.as_ptr(),
            dummy_arg_ptrs.len(),
            dummy_lib.as_ptr()
        );
    }
}
