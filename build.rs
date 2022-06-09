use std::env;
use std::path::Path;
use cmake::Config;

fn main() {
    let mut dst = Config::new("cmake")
        .out_dir(
            Path::new(&env::var("OUT_DIR").unwrap()).parent().unwrap()
                                                    .parent().unwrap()
                                                    .parent().unwrap()
        ).build();
    dst.push("lib");

    println!("cargo:rustc-link-search=native={}", dst.display());
    println!("cargo:rustc-link-lib=codeinjector");
    println!("cargo:rustc-link-lib=samplerpreload-utils");
    println!("cargo:rustc-link-lib=stdc++");
}