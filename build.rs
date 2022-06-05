use cmake::Config;

fn main() {
    let mut dst = Config::new("cmake").build();
    dst.push("lib");

    println!("cargo:rustc-link-search=native={}", dst.display());
    println!("cargo:rustc-link-lib=codeinjector");
    println!("cargo:rustc-link-lib=stdc++");
}