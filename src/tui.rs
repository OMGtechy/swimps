use crate::args::Args;

use crossterm::{terminal::{enable_raw_mode, EnterAlternateScreen, disable_raw_mode, LeaveAlternateScreen}, execute, event::{self, Event::Key, KeyCode, KeyModifiers}};
use std::time::Duration;
use tui::{Terminal, backend::CrosstermBackend, widgets::{Block, Borders}};

pub fn run(args: Args) {
    enable_raw_mode().expect("Cannot enable raw mode");
    let mut stdout = std::io::stdout();
    execute!(stdout, EnterAlternateScreen).expect("Cannot enter alternate screen");
    let mut terminal = Terminal::new(
        CrosstermBackend::new(stdout)
    ).expect("Cannot create terminal");

    loop {
        terminal.draw(|frame| {
            frame.render_widget(
                Block::default().title(format!(" swimps ─ {} ─ {} ", args.target_program(), args.trace_file())).borders(Borders::ALL),
                frame.size()
            );
        }).expect("Cannot draw TUI");

        if event::poll(Duration::from_secs(1)).expect("Cannot poll events") {
            if let Key(key_event) = event::read().expect("Cannot read events") {
                match key_event.code {
                    KeyCode::Char('d') | KeyCode::Char('c') if key_event.modifiers.contains(KeyModifiers::CONTROL) => break,
                    KeyCode::Char('q') => break,
                    _ => (),
                }
            }
        }
    }

    disable_raw_mode().expect("Cannot disable raw mode");
    execute!(terminal.backend_mut(), LeaveAlternateScreen).expect("Cannot leave alternate screen");
    terminal.show_cursor().expect("Cannot show cursor");
}