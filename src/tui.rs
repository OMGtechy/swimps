use crate::{args::Args, analysis::backtrace_by_frequency, trace::{raw::trace::Trace as RawTrace, optimised::trace::Trace as OptimisedTrace}};

use crossterm::{terminal::{enable_raw_mode, EnterAlternateScreen, disable_raw_mode, LeaveAlternateScreen}, execute, event::{self, Event::Key, KeyCode, KeyModifiers}};
use std::time::Duration;
use tui::{Terminal, backend::CrosstermBackend, widgets::{Block, Borders, Row, Table, TableState}, layout::Constraint};

pub fn run(args: Args) {
    let raw_trace = RawTrace::from(
        std::fs::read(
            args.trace_file()
        ).unwrap_or_else(|_| panic!("Could not read trace file {}", args.trace_file()))
    );

    println!("{:?}\n- {:?}\n", raw_trace.samples().iter().map(|s| s.backtrace.0.len()).collect::<Vec<_>>(), raw_trace);

    let optimised_trace = OptimisedTrace::new(raw_trace);

    println!("{:?}\n- {:?}\n", optimised_trace.stack_frames.len(), optimised_trace);

    let bbf = backtrace_by_frequency(&optimised_trace);

    println!("bbf {:?}", bbf);

    enable_raw_mode().expect("Cannot enable raw mode");
    let mut stdout = std::io::stdout();
    execute!(stdout, EnterAlternateScreen).expect("Cannot enter alternate screen");
    let mut terminal = Terminal::new(
        CrosstermBackend::new(stdout)
    ).expect("Cannot create terminal");

    let mut table_state = TableState::default();
    table_state.select(Some(0));

    loop {
        terminal.draw(|frame| {

            let table = Table::new(
                bbf.iter()
                   .map(|(k, v)| Row::new(vec![k.0.to_string(), v.to_string()]))
                   .collect::<Vec<_>>()
            )
            .header(Row::new(vec!["Backtrace ID", "Samples"].into_iter()))
            .block(Block::default()
                .title(format!(" swimps ─ {} ─ {} ", args.target_program(), args.trace_file()))
                .borders(Borders::ALL),
            )
            .widths(&[
                Constraint::Ratio(1, 2),
                Constraint::Ratio(1, 2),
            ])
            .highlight_symbol("> ");

            frame.render_stateful_widget(
                table,
                frame.size(),
                &mut table_state
            );
        }).expect("Cannot draw TUI");

        if event::poll(Duration::from_secs(1)).expect("Cannot poll events") {
            if let Key(key_event) = event::read().expect("Cannot read events") {
                match key_event.code {
                    KeyCode::Char('d') | KeyCode::Char('c') if key_event.modifiers.contains(KeyModifiers::CONTROL) => break,
                    KeyCode::Char('q') => break,
                    KeyCode::Up => table_state.select(table_state.selected().map(|n| n.saturating_sub(1))),
                    // TODO: will want to limit this to number of rows once real data is in
                    KeyCode::Down => table_state.select(table_state.selected().map(|n| n.saturating_add(1))),
                    _ => (),
                }
            }
        }
    }

    disable_raw_mode().expect("Cannot disable raw mode");
    execute!(terminal.backend_mut(), LeaveAlternateScreen).expect("Cannot leave alternate screen");
    terminal.show_cursor().expect("Cannot show cursor");
}