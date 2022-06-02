use clap::Parser;

#[derive(Parser, Debug)]
pub struct Args {
    #[clap(long)]
    trace_file: String,

    #[clap()]
    target_program: String
}

impl Args {
    pub fn trace_file(&self) -> &str {
        &self.trace_file
    }

    pub fn target_program(&self) -> &str {
        &self.target_program
    }
}

// TODO: should this go in another file?
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn trace_file_and_program() {
        let args = Args::parse_from([
            "test",
            "--trace-file",
            "hi",
            "echo"
        ].iter());
    
        assert_eq!("hi", args.trace_file());
        assert_eq!("echo", args.target_program());
    }
}
