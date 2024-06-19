#include "parser.hpp"

#include <map>

using namespace examples;

namespace {

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
    
    static bool isIDBegin(char c) {
        return std::isalpha((int)c) || c == '_';
    }
    
    static bool isID(char c) {
        return isIDBegin(c) || std::isdigit((int)c);
    }
    
    Token next() {
        skipWhitespace();
        if (text.empty()) {
            return Token{ {}, TokenKind::End };
        }
        char const* first = &text.front();
        if (isIDBegin(*first)) {
            inc();
            while (!text.empty() && isID(text.front())) {
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

}

DynUniquePtr<Program> examples::parse(std::string source) {
    Parser parser(source);
    return parser.parse();
}
