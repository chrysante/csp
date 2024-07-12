#include "interpreter.hpp"

#include <cmath>
#include <map>
#include <stdexcept>

using namespace examples;
using namespace csp;

struct Interpreter::Impl {
public:
    Impl(InterpreterDelegate& delegate): delegate(delegate) {}

    void run(Program const& prog) {
        for (auto* stmt : prog.statements()) {
            interpret(*stmt);
        }
    }

    void interpret(Statement const& stmt) {
        visit(stmt, [&](auto& stmt) { doInterpret(stmt); });
    }

    void doInterpret(EmptyStatement const&) {}

    void doInterpret(VarDecl const& decl) {
        IDMap[decl.name()->value()] = eval(*decl.initExpr());
    }

    void doInterpret(InstrStatement const& stmt) {
        using enum InstrStatement::Instruction;
        switch (stmt.instruction()) {
        case Print: {
            for (auto* arg : stmt.operands()) {
                delegate.print(eval(*arg));
            }
            break;
        }
        case Quit: delegate.quit(); break;
        }
    }

    void doInterpret(ExprStatement const& stmt) {
        double value = eval(*stmt.expr());
        delegate.eval(value);
    }

    double eval(Expr const& expr) {
        return visit(expr, [&](auto& expr) { return doEval(expr); });
    }

    double doEval(Identifier const& ID) {
        auto itr = IDMap.find(ID.value());
        if (itr == IDMap.end()) {
            throw std::runtime_error("Use of undeclared identifier: " +
                                     ID.value());
        }
        return itr->second;
    }

    double doEval(Literal const& lit) { return lit.value(); }

    double doEval(BinaryExpr const& expr) {
        double lhs = eval(*expr.lhs());
        double rhs = eval(*expr.rhs());
        switch (expr.getOperator()) {
            using enum BinaryExpr::Operator;
        case Add: return lhs + rhs;
        case Sub: return lhs - rhs;
        case Mul: return lhs * rhs;
        case Div: return lhs / rhs;
        case Pow: return std::pow(lhs, rhs);
        }
    }

    double doEval(UnaryExpr const& expr) {
        double operand = eval(*expr.operand());
        switch (expr.getOperator()) {
            using enum UnaryExpr::Operator;
        case Promote: return operand;
        case Negate: return -operand;
        }
    }

    template <int N>
    double callImpl(CallExpr const& call, std::string_view functionName,
                    auto fn) {
        if (call.arguments().size() != N) {
            throw std::runtime_error("Invalid number of arguments");
        }
        return [&]<size_t... I>(std::index_sequence<I...>) {
            return fn(eval(*call.arguments()[I])...);
        }(std::make_index_sequence<(size_t)N>{});
    }

    double doEval(CallExpr const& expr) {
        auto* ID = dyncast<Identifier const*>(expr.callee());
        if (!ID) {
            throw std::runtime_error("Cannot call expression");
        }
#define IMPL(name, numArgs, nativeName)                                        \
    if (ID->value() == name) {                                                 \
        return callImpl<numArgs>(expr, ID->value(), [](auto... args) {         \
            return nativeName(args...);                                        \
        });                                                                    \
    }
        IMPL("sqrt", 1, std::sqrt);
        IMPL("pow", 2, std::pow);
        IMPL("exp", 1, std::exp);
        IMPL("exp2", 1, std::exp2);
        IMPL("log", 1, std::log);
        IMPL("log2", 1, std::log2);
        throw std::runtime_error("Use of unknown function: " + ID->value());
    }

    InterpreterDelegate& delegate;
    std::map<std::string, double> IDMap;
};

Interpreter::Interpreter(InterpreterDelegate& delegate):
    impl(std::make_unique<Impl>(delegate)) {}

Interpreter::~Interpreter() = default;

void Interpreter::run(Program const& program) {
    impl->run(program);
}
