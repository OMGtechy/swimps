use cmake::Config;

fn main() {
    Config::new("cmake").build_target("all").build();
}