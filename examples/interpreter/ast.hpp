#ifndef AST_HPP
#define AST_HPP

/// This file defines AST nodes that we will use to evaluate expressions

#include <algorithm>
#include <cassert>
#include <concepts>
#include <memory>
#include <ranges>
#include <string>
#include <vector>

#include "csp.hpp"

/// X-Macro (https://en.wikipedia.org/wiki/X_macro) that serves as a single
/// source of truth for the lists we are going to define
#define AST_LIST(X)                                                            \
    X(ASTNode, NoParent, Abstract)                                             \
    X(Expr, ASTNode, Abstract)                                                 \
    X(Identifier, Expr, Concrete)                                              \
    X(Literal, Expr, Concrete)                                                 \
    X(BinaryExpr, Expr, Concrete)                                              \
    X(UnaryExpr, Expr, Concrete)                                               \
    X(CallExpr, Expr, Concrete)                                                \
    X(Statement, ASTNode, Abstract)                                            \
    X(EmptyStatement, Statement, Concrete)                                     \
    X(VarDecl, Statement, Concrete)                                            \
    X(InstrStatement, Statement, Concrete)                                     \
    X(ExprStatement, Statement, Concrete)                                      \
    X(Program, ASTNode, Concrete)

namespace examples {

#define X(Name, ...) class Name;
AST_LIST(X)
#undef X

enum class ASTNodeID {
#define X(Name, ...) Name,
    AST_LIST(X)
#undef X
};

using NoParent = void;

} // namespace examples

#define X(Name, Parent, Corporeality)                                          \
    CSP_DEFINE(examples::Name, examples::ASTNodeID::Name, examples::Parent,    \
               Corporeality)
AST_LIST(X)
#undef X

namespace examples {

template <typename T>
using DynUniquePtr = std::unique_ptr<T, csp::dyn_deleter>;

class ASTNode: public csp::base_helper<ASTNode> {
public:
    template <typename T = ASTNode>
    T* childAt(size_t index) const {
        assert(index < numChildren());
        return csp::cast<T*>(m_children[index].get());
    }

    size_t numChildren() const { return m_children.size(); }

    auto children() const {
        return m_children |
               std::views::transform([](auto& p) { return p.get(); });
    }

protected:
    ASTNode(ASTNodeID ID, std::vector<DynUniquePtr<ASTNode>> children):
        base_helper(ID), m_children(std::move(children)) {}

    template <typename... Children>
    ASTNode(ASTNodeID ID, Children&&... children): ASTNode(ID, {}) {
        (m_children.push_back(std::forward<Children>(children)), ...);
    }

private:
    std::vector<DynUniquePtr<ASTNode>> m_children;
};

class Expr: public ASTNode {
protected:
    using ASTNode::ASTNode;
};

class Identifier: public Expr {
public:
    explicit Identifier(std::string value):
        Expr(ASTNodeID::Identifier), m_value(std::move(value)) {}

    std::string value() const { return m_value; }

private:
    std::string m_value;
};

class Literal: public Expr {
public:
    explicit Literal(double value): Expr(ASTNodeID::Literal), m_value(value) {}

    double value() const { return m_value; }

private:
    double m_value;
};

class BinaryExpr: public Expr {
public:
    enum Operator { Add, Sub, Mul, Div, Pow };

    explicit BinaryExpr(Operator op, DynUniquePtr<Expr> lhs,
                        DynUniquePtr<Expr> rhs):
        Expr(ASTNodeID::BinaryExpr, std::move(lhs), std::move(rhs)),
        m_operator(op) {}

    Operator getOperator() const { return m_operator; }

    Expr* lhs() const { return childAt<Expr>(0); }

    Expr* rhs() const { return childAt<Expr>(1); }

private:
    Operator m_operator;
};

class UnaryExpr: public Expr {
public:
    enum Operator { Promote, Negate };

    explicit UnaryExpr(Operator op, DynUniquePtr<Expr> operand):
        Expr(ASTNodeID::UnaryExpr, std::move(operand)), m_operator(op) {}

    Operator getOperator() const { return m_operator; }

    Expr* operand() const { return childAt<Expr>(0); }

private:
    Operator m_operator;
};

class CallExpr: public Expr {
public:
    explicit CallExpr(DynUniquePtr<Expr> callee,
                      std::vector<DynUniquePtr<Expr>> arguments):
        Expr(ASTNodeID::CallExpr,
             makeChildren(std::move(callee), std::move(arguments))) {}

    Expr* callee() const { return childAt<Expr>(0); }

    auto arguments() const {
        return children() | std::views::drop(1) |
               std::views::transform(csp::cast<Expr*>);
    }

private:
    static std::vector<DynUniquePtr<ASTNode>>
    makeChildren(DynUniquePtr<Expr> callee,
                 std::vector<DynUniquePtr<Expr>> arguments) {
        std::vector<DynUniquePtr<ASTNode>> children;
        children.reserve(arguments.size() + 1);
        children.push_back(std::move(callee));
        children.insert(children.end(), std::move_iterator(arguments.begin()),
                        std::move_iterator(arguments.end()));
        return children;
    }
};

class Statement: public ASTNode {
protected:
    using ASTNode::ASTNode;
};

class EmptyStatement: public Statement {
public:
    EmptyStatement(): Statement(ASTNodeID::EmptyStatement) {}
};

class VarDecl: public Statement {
public:
    explicit VarDecl(DynUniquePtr<Identifier> name,
                     DynUniquePtr<Expr> initExpr):
        Statement(ASTNodeID::VarDecl, std::move(name), std::move(initExpr)) {}

    Identifier* name() const { return childAt<Identifier>(0); }

    Expr* initExpr() const { return childAt<Expr>(1); }
};

class InstrStatement: public Statement {
public:
    enum Instruction { Print, Quit };

    explicit InstrStatement(Instruction instr,
                            std::vector<DynUniquePtr<ASTNode>> operands):
        Statement(ASTNodeID::InstrStatement, std::move(operands)),
        m_instr(instr) {}

    Instruction instruction() const { return m_instr; }

    auto operands() const {
        return children() | std::views::transform(csp::cast<Expr*>);
    }

    Expr* initExpr() const { return childAt<Expr>(1); }

private:
    Instruction m_instr;
};

class ExprStatement: public Statement {
public:
    explicit ExprStatement(DynUniquePtr<Expr> expr):
        Statement(ASTNodeID::ExprStatement, std::move(expr)) {}

    Expr* expr() const { return childAt<Expr>(0); }
};

class Program: public ASTNode {
public:
    explicit Program(std::vector<DynUniquePtr<ASTNode>> children):
        ASTNode(ASTNodeID::Program, std::move(children)) {}

    auto statements() const {
        return children() | std::views::transform(csp::cast<Statement*>);
    }
};

} // namespace examples

#endif // AST_HPP
