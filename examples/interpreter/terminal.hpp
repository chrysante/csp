#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <string>

namespace examples {

struct TerminalDelegate {
    virtual void onInput(std::string input) {}

    virtual bool complete(std::string& input) { return false; }
};

int runTerminal(TerminalDelegate& delegate);

} // namespace examples

#endif // TERMINAL_HPP
