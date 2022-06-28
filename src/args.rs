use clap::Parser;

#[derive(Parser, Debug)]
pub struct Args {
    #[clap(long)]
    trace_file: String,

    #[clap(long)]
    tui: bool,

    #[clap()]
    target_program: String,

    #[clap()]
    target_program_args: Vec<String>
}

impl Args {
    pub fn trace_file(&self) -> &str {
        &self.trace_file
    }

    pub fn tui(&self) -> bool {
        self.tui
    }

    pub fn target_program(&self) -> &str {
        &self.target_program
    }

    pub fn target_program_args(&self) -> &Vec<String> {
        &self.target_program_args
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
        assert!(!args.tui());
    }

    #[test]
    fn trace_file_and_program_and_args() {
        let args = Args::parse_from([
            "test",
            "--trace-file",
            "hi",
            "echo",
            "bingo!",
            "bango..."
        ].iter());
    
        assert_eq!("hi", args.trace_file());
        assert_eq!("echo", args.target_program());

        let target_program_args = args.target_program_args();
        assert_eq!(2, target_program_args.len());
        assert_eq!("bingo!", target_program_args[0]);
        assert_eq!("bango...", target_program_args[1]);
        assert!(!args.tui());
    }

    #[test]
    fn tui() {
        let args = Args::parse_from([
            "test",
            "--trace-file",
            "foobar",
            "--tui",
            "blah"
        ]);

        assert_eq!("foobar", args.trace_file());
        assert_eq!("blah", args.target_program());
        assert!(args.tui());
    }
}
