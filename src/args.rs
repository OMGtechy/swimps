use clap::Parser;

#[derive(Parser, Debug)]
pub struct Args {
    #[clap(long)]
    trace_file: String
}

impl Args {
    pub fn trace_file(&self) -> &str {
        &self.trace_file
    }
}

// TODO: should this go in another file?
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn trace_file() {
        assert_eq!("hi", Args::parse_from(["test", "--trace-file", "hi"].iter()).trace_file());
    }
}
