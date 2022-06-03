pub mod args;

fn main() {
    let args = args::Args::parse(); 
    println!("Trace file: {:?}", args);
}
