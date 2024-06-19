#include "interpreter.hpp"

#include <cmath>
#include <map>
#include <stdexcept>

using namespace examples;
using namespace fastvis;

struct Interpreter::Impl {
public:
    Impl(InterpreterDelegate& delegate): delegate(delegate) {}
    
    void run(Program const& prog) {
        for (auto* stmt : prog.statements()) {
            interpret(*stmt);
        }
    }
    
    void interpret(Statement const& stmt) {
        fastvis::visit(stmt, [&](auto& stmt) { doInterpret(stmt); });
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
        case Quit:
            delegate.quit();
            break;
        }
    }
    
    void doInterpret(ExprStatement const& stmt) {
        double value = eval(*stmt.expr());
        delegate.eval(value);
    }
    
    double eval(Expr const& expr) {
        return fastvis::visit(expr, [&](auto& expr) { return doEval(expr); });
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
    
    double doEval(CallExpr const& expr) {
        auto* ID = fastvis::dyncast<Identifier const*>(expr.callee());
        if (!ID) {
            throw std::runtime_error("Cannot call expression");
        }
        if (ID->value() == "sqrt") {
            if (expr.arguments().size() != 1) {
                throw std::runtime_error(
                                         "Invalid number of arguments for 'sqrt'");
            }
            return std::sqrt(eval(*expr.arguments().front()));
        }
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
