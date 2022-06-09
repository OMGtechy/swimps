use clap::Parser;

#[derive(Parser, Debug)]
pub struct Args {
    #[clap(long)]
    trace_file: String,

    #[clap()]
    target_program: String,

    #[clap()]
    target_program_args: Option<String>
}

impl Args {
    pub fn trace_file(&self) -> &str {
        &self.trace_file
    }

    pub fn target_program(&self) -> &str {
        &self.target_program
    }

    pub fn target_program_args(&self) -> Option<&str> {
        match &self.target_program_args {
            Some(s) => Some(s),
            None => None
        }
    }

    pub fn parse() -> Args {
        Parser::parse()
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

    #[test]
    fn trace_file_and_program_and_args() {
        let args = Args::parse_from([
            "test",
            "--trace-file",
            "hi",
            "echo",
            "bingo!"
        ].iter());
    
        assert_eq!("hi", args.trace_file());
        assert_eq!("echo", args.target_program());
        assert_eq!(Some("bingo!"), args.target_program_args());
    }
}
