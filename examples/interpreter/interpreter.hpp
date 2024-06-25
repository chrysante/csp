#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <memory>

#include "ast.hpp"

namespace examples {

class InterpreterDelegate {
public:
    virtual void print(double value) = 0;

    virtual void eval(double value) = 0;

    virtual void quit() = 0;
};

class Interpreter {
public:
    explicit Interpreter(InterpreterDelegate& delegate);

    ~Interpreter();

    void run(Program const& program);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace examples

#endif // INTERPRETER_HPP
