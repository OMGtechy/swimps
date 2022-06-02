pub mod args;

// TODO: this shouldn't be necessary,
//       what happened to encapsulation?
use clap::Parser;

fn main() {
    let args = args::Args::parse(); 
    println!("Trace file: {:?}", args);
}
