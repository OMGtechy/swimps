#include "swimps-tui.h"

#include <optional>
#include <limits>
#include <map>

#include <ncurses.h>

#include "swimps-assert.h"

using swimps::analysis::Analysis;
using CallTreeNode = Analysis::CallTreeNode;
using swimps::error::ErrorCode;
using swimps::trace::stack_frame_count_t;
using swimps::trace::stack_frame_id_t;
using swimps::trace::StackFrame;
using swimps::trace::Trace;

namespace {
    // We could have as many lines as stack frames, worst case.
    using line_t = stack_frame_count_t;

    using expansion_state_t = std::map<const CallTreeNode*, bool>;
    using line_mappings_t = std::map<line_t, const CallTreeNode*>;

    const StackFrame* lookup_stack_frame(const Trace& trace, const stack_frame_id_t id) {
        const auto& stackFrames = trace.stackFrames;
        const auto iter = std::find_if(stackFrames.cbegin(), stackFrames.cend(), [id](const auto& stackFrame) { return stackFrame.id == id; });
        return iter != stackFrames.cend() ? &(*iter) : nullptr;
    };

    void print_node(WINDOW* const window,
                    const Trace& trace,
                    const CallTreeNode& rootNode,
                    expansion_state_t& expansionState,
                    line_mappings_t& lineMappings,
                    const line_t selectedLine,
                    const line_t linesToSkip,
                    line_t& currentLine,
                    const std::size_t indentation) {

        if (currentLine >= linesToSkip) {
            for(std::size_t i = 0; i < indentation; ++i) {
                wprintw(window, "    ");
            }

            const auto* const stackFrame = lookup_stack_frame(trace, rootNode.stackFrameID);
            wprintw(
                window,
                "%s %s %s\n",
                selectedLine == currentLine ? "->" : "  ",
                rootNode.children.size() == 0 ? "   " : expansionState[&rootNode] ? "[-]" : "[+]",
                stackFrame != nullptr ? stackFrame->mangledFunctionName : "?"
            );
        }

        lineMappings[currentLine] = &rootNode;
        currentLine += 1;

        if (expansionState[&rootNode]) {
            for(const auto& childNode : rootNode.children) {
                print_node(
                    window,
                    trace,
                    childNode,
                    expansionState,
                    lineMappings,
                    selectedLine,
                    linesToSkip,
                    currentLine,
                    indentation + 1
                );
            }
        }
    }
}

ErrorCode swimps::tui::run(const Trace& trace, const Analysis& analysis) {
    WINDOW* const window = initscr();
    swimps_assert(window != nullptr);
    keypad(window, true);

    expansion_state_t expansionState;
    line_mappings_t lineMappings;

    line_t selectedLine = 0;
    line_t currentLine = 0;

    const auto printCallTree = [window, &currentLine, &lineMappings, &selectedLine, &expansionState, &trace, &analysis](const stack_frame_count_t linesToSkip){
        for(const auto& rootNode : analysis.callTree) {
            print_node(
                window,
                trace,
                rootNode,
                expansionState,
                lineMappings,
                selectedLine,
                linesToSkip,
                currentLine,
                0
            );
        }
    };

    line_t callTreeOffset = 0;

    bool quit = false;
    while(!quit) {
        werase(window);
        currentLine = 0;
        printCallTree(callTreeOffset);
        wrefresh(window);
        const int input = wgetch(window);
        switch(input) {
        case 'w':
        case KEY_UP:
            if (selectedLine > 0) {
                selectedLine -= 1;
            }
            break;
        case 's':
        case KEY_DOWN:
            if (selectedLine < std::numeric_limits<decltype(selectedLine)>::max()) {
                selectedLine += 1;
            }
            break;
        case KEY_LEFT:
        case KEY_RIGHT:
            {
                const auto lineMappingIter = lineMappings.find(selectedLine);
                if (lineMappingIter != lineMappings.end()) {
                    const auto expansionStateIter = expansionState.find(lineMappingIter->second);
                    if (expansionStateIter != expansionState.end()) {
                        expansionStateIter->second = input == KEY_RIGHT;
                    }
                }
            }
            break;
        case 'q':
            quit = true;
            break; 
        default:
            break;
        }
    }

    endwin();

    return ErrorCode::None;
}
