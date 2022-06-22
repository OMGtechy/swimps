pub mod args;

use std::ffi::CString;
use std::os::raw::{c_char, c_void, c_double};

extern "C" {
    fn codeinjector_inject_library(program: *const c_char, args: *const *const c_char, argsCount: usize, lib: *const c_char) -> bool;

    fn samplerpreload_settings_ctor() -> *const c_void;
    fn samplerpreload_settings_dtor(instance: *const c_void);
    fn samplerpreload_settings_set_samples_per_second(instance: *const c_void, samplesPerSecond: c_double);
    fn samplerpreload_settings_set_trace_file_path(instance: *const c_void, traceFilePath: *const c_char);
    fn samplerpreload_settings_write_to_env(instance: *const c_void);
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

    let trace_file_path = CString::new(args.trace_file()).unwrap(); 

    unsafe {
        let settings = samplerpreload_settings_ctor();
        samplerpreload_settings_set_samples_per_second(settings, 1.0);
        samplerpreload_settings_set_trace_file_path(settings, trace_file_path.as_ptr());
        samplerpreload_settings_write_to_env(settings);
        samplerpreload_settings_dtor(settings);

        codeinjector_inject_library(
            program.as_ptr(),
            dummy_arg_ptrs.as_ptr(),
            dummy_arg_ptrs.len(),
            lib.as_ptr()
        );
    }
}
