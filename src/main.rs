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
    let program = CString::new(args.target_program()).unwrap();

    let mut lib = std::env::current_exe().unwrap();
    lib.pop();
    lib.push("lib");
    lib.push("libsamplerpreload.so");
    let lib = CString::new(lib.as_path().to_str().unwrap()).unwrap();
    println!("{:?}", lib);

    unsafe {
        codeinjector_inject_library(
            program.as_ptr(),
            dummy_arg_ptrs.as_ptr(),
            dummy_arg_ptrs.len(),
            lib.as_ptr()
        );
    }
}
