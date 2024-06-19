#include "terminal.hpp"

#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>
#include <cassert>

#include <termios.h>

#include "utils.hpp"

/// https://stackoverflow.com/a/21101030/21285803

using namespace examples;

namespace {

struct InputHistory {
    void setCurrent(std::string line) {
        lines.back() = std::move(line);
    }
    
    void push(std::string line) {
        lines.back() = std::move(line);
        index = lines.size();
        lines.push_back("");
    }
    
    std::string getLast() {
        if (index > 0) {
            --index;
        }
        return lines[index];
    }
    
    std::string getPrev() {
        if (index < lines.size() - 1) {
            ++index;
        }
        return lines[index];
    }
    
    std::vector<std::string> lines = {""};
    std::size_t index = 0;
};

struct Terminal {
    explicit Terminal(TerminalDelegate& delegate): delegate(delegate) {
        tcgetattr(0, &info);          /* get current terminal attirbutes; 0 is the file descriptor for stdin */
        info.c_lflag &= ~ICANON;      /* disable canonical mode */
        info.c_cc[VMIN] = 1;          /* wait until at least one keystroke available */
        info.c_cc[VTIME] = 0;         /* no timeout */
        tcsetattr(0, TCSANOW, &info); /* set immediately */
    }
    
    ~Terminal() {
        tcgetattr(0, &info);
        info.c_lflag |= ICANON;
        tcsetattr(0, TCSANOW, &info);
    }
    
    static void beginInput() {
        clearLine();
        format(Format::Blue, Format::Bold);
        std::cout << "> ";
        format(Format::Reset);
    }
    
    void run() {
        beginInput();
        while(true) {
            int ch = getchar();
            if (ch < 0) {
                if (ferror(stdin)) {
                    std::cerr << "Error\n";
                }
                clearerr(stdin);
                continue;
            }
            clearLine();
#if 0
            std::cout << ch << std::endl;
            continue;
#endif
            handleInput(ch);
            beginInput();
            printf("%s", inputBuffer.c_str());
            if (position < (int)inputBuffer.size()) {
                printf("\033[%dD", (int)inputBuffer.size() - position);
            }
        }
    }
    
    enum Codes {
        LeftArrow = 68,
        RightArrow = 67,
        UpArrow = 65,
        DownArrow = 66,
        Enter = 10,
        Escape = 27,
        Backspace = 127,
        Tab = 9,
    };
    
    void handleInput(int input) {
        if (input == Escape) {
            escape = 2;
            return;
        }
        if (!escape) {
            if (input >= 32 && input < 127) {
                inputBuffer.insert(inputBuffer.begin() + position++, input);
                history.setCurrent(inputBuffer);
            }
            if (input == Backspace && position > 0) {
                inputBuffer.erase(inputBuffer.begin() + --position);
            }
            if (input == Enter) {
                history.push(inputBuffer);
                auto copy = std::move(inputBuffer);
                inputBuffer.clear();
                position = 0;
                delegate.onInput(std::move(copy));
            }
            if (input == Tab && delegate.complete(inputBuffer)) {
                history.setCurrent(inputBuffer);
                position = (int)inputBuffer.size();
            }
            return;
        }
        if (escape == 2) {
            --escape;
            return;
        }
        if (escape == 1) {
            --escape;
        }
        assert(escape == 0);
        switch (input) {
        case LeftArrow:
            if (position > 0) {
                --position;
            }
            break;
        case RightArrow:
            if (position < (int)inputBuffer.size()) {
                ++position;
            }
            break;
        case UpArrow:
            inputBuffer = history.getLast();
            position = (int)inputBuffer.size();
            break;
        case DownArrow:
            inputBuffer = history.getPrev();
            position = (int)inputBuffer.size();
            break;
        default:
            break;
        }
    }
    
    static void clearLine() {
        printf("\33[2K\r");
    }
    
    TerminalDelegate& delegate;
    termios info;
    std::string inputBuffer;
    int position = 0;
    int escape = 0;
    InputHistory history;
};

}

int examples::runTerminal(TerminalDelegate& delegate) {
    Terminal terminal(delegate);
    try {
        terminal.run();
    }
    catch (QuitException const&) {
        return EXIT_SUCCESS;
    }
    catch (InputException const& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
