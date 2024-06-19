#include <cmath>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <string_view>

#include "parser.hpp"

using namespace examples;
using namespace fastvis;

enum class TokenKind {
    Let,
    Print,
    Quit,
    Identifier,
    NumericLiteral,
    CloseParen,
    Semicolon,
    Comma,
    Assign,
    Add,
    Sub,
    Mul,
    Div,
    OpenParen,
    End
};

struct Token {
    std::string_view ID;
    TokenKind kind;
};

class Lexer {
    std::string_view text;

    void skipWhitespace() {
        while (!text.empty() && std::isspace((int)text.front())) {
            inc();
        }
    }

    void inc() {
        assert(!text.empty());
        text = text.substr(1);
    }

    std::string_view getID(char const* begin) const {
        return std::string_view(begin, std::to_address(text.begin()));
    }

    inline static std::map<std::string_view, TokenKind> const keywords = {
        { "let", TokenKind::Let },
        { "print", TokenKind::Print },
        { "quit", TokenKind::Quit }
    };

public:
    explicit Lexer(std::string_view text): text(text) {}

    Token next() {
        skipWhitespace();
        if (text.empty()) {
            return Token{ {}, TokenKind::End };
        }
        char const* first = &text.front();
        if (std::isalpha((int)*first)) {
            inc();
            while (!text.empty() && std::isalnum((int)text.front())) {
                inc();
            }
            std::string_view ID = getID(first);
            if (auto itr = keywords.find(ID); itr != keywords.end()) {
                return Token{ ID, itr->second };
            }
            return Token{ ID, TokenKind::Identifier };
        }
        if ((std::isdigit((int)*first))) {
            while (!text.empty() && std::isdigit((int)text.front())) {
                inc();
            }
            return Token{ getID(first), TokenKind::NumericLiteral };
        }
        auto matchChar = [&](char op,
                             TokenKind tokKind) -> std::optional<Token> {
            if (*first == op) {
                inc();
                return Token{ getID(first), tokKind };
            }
            return std::nullopt;
        };
        if (auto tok = matchChar('(', TokenKind::OpenParen))
            return *tok;
        if (auto tok = matchChar(')', TokenKind::CloseParen))
            return *tok;
        if (auto tok = matchChar(';', TokenKind::Semicolon))
            return *tok;
        if (auto tok = matchChar(',', TokenKind::Comma))
            return *tok;
        if (auto tok = matchChar('=', TokenKind::Assign))
            return *tok;
        if (auto tok = matchChar('+', TokenKind::Add))
            return *tok;
        if (auto tok = matchChar('-', TokenKind::Sub))
            return *tok;
        if (auto tok = matchChar('*', TokenKind::Mul))
            return *tok;
        if (auto tok = matchChar('/', TokenKind::Div))
            return *tok;
        throw std::runtime_error("Failed to scan token");
    }
};

template <typename T, typename... Args>
requires std::constructible_from<T, Args...>
DynUniquePtr<T> allocate(Args&&... args) {
    return DynUniquePtr<T>(new T(std::forward<Args>(args)...));
}

class Parser {
public:
    explicit Parser(Lexer lexer): lexer(lexer) {}

    explicit Parser(std::string_view text): lexer(text) {}

    DynUniquePtr<Program> parse() {
        std::vector<DynUniquePtr<ASTNode>> statements;
        while (true) {
            if (auto stmt = parseStmt()) {
                statements.push_back(std::move(stmt));
            }
            else {
                return allocate<Program>(std::move(statements));
            }
        }
    }

private:
    DynUniquePtr<Expr> parseExpr() { return parseBinaryExpr(); }

    static std::optional<BinaryExpr::Operator> toBinOp(TokenKind kind) {
        switch (kind) {
        case TokenKind::Add: return BinaryExpr::Add;
        case TokenKind::Sub: return BinaryExpr::Sub;
        case TokenKind::Mul: return BinaryExpr::Mul;
        case TokenKind::Div: return BinaryExpr::Div;
        default: return std::nullopt;
        }
    }

    DynUniquePtr<Expr> parseBinaryExpr() {
        auto lhs = parseUnaryExpr();
        while (true) {
            if (auto op = toBinOp(peek().kind)) {
                eat();
                auto rhs = parseExpr();
                expect(rhs.get(), "expression");
                lhs = allocate<BinaryExpr>(*op, std::move(lhs), std::move(rhs));
            }
            return lhs;
        }
    }

    static std::optional<UnaryExpr::Operator> toUnOp(TokenKind kind) {
        switch (kind) {
        case TokenKind::Add: return UnaryExpr::Promote;
        case TokenKind::Sub: return UnaryExpr::Negate;
        default: return std::nullopt;
        }
    }

    DynUniquePtr<Expr> parseUnaryExpr() {
        if (auto op = toUnOp(peek().kind)) {
            eat();
            auto operand = parseExpr();
            expect(operand.get(), "expression");
            return allocate<UnaryExpr>(*op, std::move(operand));
        }
        return parseCallExpr();
    }

    template <typename T = Expr>
    std::vector<DynUniquePtr<T>> parseArgumentList(TokenKind delim) {
        std::vector<DynUniquePtr<T>> arguments;
        bool first = true;
        while (true) {
            if (peek().kind == delim) {
                eat();
                return arguments;
            }
            if (!first) {
                expect(eat(), TokenKind::Comma);
            }
            first = false;
            auto arg = parseExpr();
            expect(arg.get(), "expression");
            arguments.push_back(std::move(arg));
        }
    }

    DynUniquePtr<Expr> parseCallExpr() {
        auto prim = parsePrimary();
        if (peek().kind != TokenKind::OpenParen) {
            return prim;
        }
        eat();
        auto arguments = parseArgumentList(TokenKind::CloseParen);
        return allocate<CallExpr>(std::move(prim), std::move(arguments));
    }

    DynUniquePtr<Expr> parsePrimary() {
        if (peek().kind == TokenKind::OpenParen) {
            eat();
            auto expr = parseExpr();
            expect(eat(), TokenKind::CloseParen);
            return expr;
        }
        if (auto expr = parseIdentifier()) {
            return expr;
        }
        if (auto expr = parseLiteral()) {
            return expr;
        }
        return nullptr;
    }

    DynUniquePtr<Identifier> parseIdentifier() {
        auto tok = peek();
        if (tok.kind == TokenKind::Identifier) {
            eat();
            return allocate<Identifier>(std::string(tok.ID));
        }
        return nullptr;
    }

    DynUniquePtr<Literal> parseLiteral() {
        auto tok = peek();
        if (tok.kind == TokenKind::NumericLiteral) {
            eat();
            return allocate<Literal>(std::stod(std::string(tok.ID)));
        }
        return nullptr;
    }

    DynUniquePtr<Statement> parseStmt() {
        if (peek().kind == TokenKind::Let) {
            return parseVarDecl();
        }
        if (auto stmt = parseExprStmt()) {
            return stmt;
        }
        if (auto stmt = parseInstrStmt()) {
            return stmt;
        }
        if (peek().kind == TokenKind::Semicolon) {
            eat();
            return allocate<EmptyStatement>();
        }
        return nullptr;
    }

    DynUniquePtr<VarDecl> parseVarDecl() {
        assert(peek().kind == TokenKind::Let);
        eat();
        auto name = parseIdentifier();
        expect(name.get(), "identifier");
        expect(eat(), TokenKind::Assign);
        auto initExpr = parseExpr();
        expect(initExpr.get(), "expression");
        expect(eat(), TokenKind::Semicolon);
        return allocate<VarDecl>(std::move(name), std::move(initExpr));
    }

    static std::optional<InstrStatement::Instruction> toInstr(TokenKind kind) {
        switch (kind) {
        case TokenKind::Print: return InstrStatement::Print;
        case TokenKind::Quit: return InstrStatement::Quit;
        default: return std::nullopt;
        }
    }

    DynUniquePtr<InstrStatement> parseInstrStmt() {
        if (auto instr = toInstr(peek().kind)) {
            eat();
            auto args = parseArgumentList<ASTNode>(TokenKind::Semicolon);
            return allocate<InstrStatement>(*instr, std::move(args));
        }
        return nullptr;
    }

    DynUniquePtr<ExprStatement> parseExprStmt() {
        auto expr = parseExpr();
        if (!expr) {
            return nullptr;
        }
        expect(eat(), TokenKind::Semicolon);
        return allocate<ExprStatement>(std::move(expr));
    }

    void expect(ASTNode const* node, std::string const& kind) {
        if (!node) {
            throw std::runtime_error("Expected " + kind);
        }
    }

    void expect(Token token, TokenKind kind) {
        if (token.kind != kind) {
            throw std::runtime_error("Invalid token");
        }
    }

    Token peek() {
        if (!current) {
            current = lexer.next();
        }
        return *current;
    }

    Token eat() {
        if (current) {
            return *std::exchange(current, std::nullopt);
        }
        return lexer.next();
    }

    Lexer lexer;
    std::optional<Token> current;
};

[[maybe_unused]] void debugPrint(ASTNode const& node,
                                 std::ostream& str = std::cout) {
    int indent = 0;
    auto getTypeName = [](ASTNode const& node) {
        return visit(node,
                     []<typename T>(T const&) { return typeid(T).name(); });
    };
    auto impl = [&](auto& impl, ASTNode const& node) -> void {
        for (int i = 0; i < indent; ++i) {
            str << "  ";
        }
        str << getTypeName(node) << "\n";
        ++indent;
        for (auto* child : node.children()) {
            impl(impl, *child);
        }
        --indent;
    };
    impl(impl, node);
}

class Interpreter {
public:
    void run(Program const& prog) {
        for (auto* stmt : prog.statements()) {
            interpret(*stmt);
        }
    }

private:
    void interpret(Statement const& stmt) {
        fastvis::visit(stmt, [&](auto& stmt) { doInterpret(stmt); });
    }

    void doInterpret(EmptyStatement const&) {}

    void doInterpret(VarDecl const& decl) {
        IDMap.insert({ decl.name()->value(), eval(*decl.initExpr()) });
    }

    void doInterpret(InstrStatement const& stmt) {
        using enum InstrStatement::Instruction;
        switch (stmt.instruction()) {
        case Print: {
            std::cout << ":: ";
            bool first = true;
            for (auto* arg : stmt.operands()) {
                if (!first) {
                    std::cout << ", ";
                }
                first = false;
                std::cout << eval(*arg);
            }
            std::cout << "\n";
            break;
        }
        case Quit: std::exit(0); break;
        }
    }

    void doInterpret(ExprStatement const& stmt) { eval(*stmt.expr()); }

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

    std::map<std::string, double> IDMap;
};

int main() {
    Interpreter interpreter;
    while (true) {
        try {
            std::cout << ">_ ";
            std::string input;
            std::getline(std::cin, input);
            input += ";";
            Parser parser(input);
            auto prog = parser.parse();
            interpreter.run(*prog);
        }
        catch (std::exception const& e) {
            std::cout << "Failed to evaluate: " << e.what() << std::endl;
        }
    }
}
