#include <iostream>

#include "ast.hpp"
#include "interpreter.hpp"
#include "parser.hpp"
#include "terminal.hpp"
#include "utils.hpp"

using namespace examples;

namespace {

struct TermDelegateImpl: TerminalDelegate {
    Interpreter& interpreter;

    explicit TermDelegateImpl(Interpreter& interpreter):
        interpreter(interpreter) {}

    void onInput(std::string input) override {
        input += ";";
        try {
            auto prog = parse(input);
            interpreter.run(*prog);
        }
        catch (std::runtime_error const& e) {
            format(Format::Red, Format::Bold);
            printf("Error: ");
            format(Format::Reset);
            printf("%s\n", e.what());
        }
    }

    bool complete(std::string& input) override { return false; }
};

struct InterpreterDelegateImpl: InterpreterDelegate {
    void print(double value) override {
        format(Format::Green, Format::Bold);
        printf(">> ");
        format(Format::Reset);
        format(Format::Bold);
        std::cout << value << std::endl;
        format(Format::Reset);
    }

    void eval(double value) override {
        format(Format::Grey, Format::Bold);
        std::cout << ">> " << value << std::endl;
        format(Format::Reset);
    }

    void quit() override { throw QuitException(); }
};

} // namespace

int main() {
    InterpreterDelegateImpl interpreterDelegate;
    Interpreter interpreter(interpreterDelegate);
    TermDelegateImpl termDelegate(interpreter);
    return runTerminal(termDelegate);
}
