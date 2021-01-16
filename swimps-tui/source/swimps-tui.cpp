#include "swimps-tui.h"

#include <functional>
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

ErrorCode swimps::tui::run(const Trace& trace, const Analysis& analysis) {
    WINDOW* const window = initscr();
    swimps_assert(window != nullptr);
    keypad(window, true);

    std::map<const CallTreeNode*, bool> expansionState;
   
    stack_frame_count_t selectedLine = 0;
    stack_frame_count_t line = 0;

    std::map<decltype(line), const CallTreeNode*> lineMappings; 

    const auto printCallTree = [window, &line, &lineMappings, &selectedLine, &expansionState, &trace, &analysis](const stack_frame_count_t linesToSkip){
        const auto lookupStackFrame = [&trace](const stack_frame_id_t id) -> const StackFrame* {
            const auto& stackFrames = trace.stackFrames;
            const auto iter = std::find_if(stackFrames.cbegin(), stackFrames.cend(), [id](const auto& stackFrame) { return stackFrame.id == id; });
            return iter != stackFrames.cend() ? &(*iter) : nullptr;
        };

        std::function<void(const CallTreeNode&, const std::size_t)> printNode;
        printNode = [&selectedLine, &expansionState, &line, &lineMappings, linesToSkip, &printNode, lookupStackFrame, window](const CallTreeNode& rootNode, const std::size_t indentation) {
            if (line >= linesToSkip) {
                for(std::size_t i = 0; i < indentation; ++i) {
                    wprintw(window, "-");
                }

                const auto* const stackFrame = lookupStackFrame(rootNode.stackFrameID);
                wprintw(window, "%c %s %s\n", selectedLine == line ? '>' : ' ', rootNode.children.size() == 0 ? "   " : expansionState[&rootNode] ? "[-]" : "[+]", stackFrame != nullptr ? stackFrame->mangledFunctionName : "?");
            }

            lineMappings[line] = &rootNode;
            line += 1;

            if (expansionState[&rootNode]) {
                for(const auto& childNode : rootNode.children) {
                   printNode(childNode, indentation + 1); 
                }
            }
        };

        for(const auto& rootNode : analysis.callTree) {
            printNode(rootNode, 0);
        }
    };

    stack_frame_count_t callTreeOffset = 0;

    bool quit = false;
    while(!quit) {
        werase(window);
        line = 0;
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
