#pragma once
#include <string>
#include <vector>
#include <memory>
#include <cmath>

class MathParser {
public:
    enum class TokenType {
        Number,
        Variable,
        Plus,
        Minus,
        Multiply,
        Divide,
        Power,
        LParen,
        RParen,
        Comma,
        Function,
        End
    };

    struct Token {
        TokenType type;
        double value = 0.0;
        std::string text;
    };

    struct Node;
    using NodePtr = std::shared_ptr<Node>;

    struct Node {
        virtual ~Node() = default;
        virtual double evaluate(double t) const = 0;
    };

    MathParser();
    explicit MathParser(const std::string& expression);

    bool setExpression(const std::string& expression);
    double evaluate(double t) const;
    bool isValid() const { return valid; }
    const std::string& getExpression() const { return exprStr; }
    const std::string& getLastError() const { return lastError; }

private:
    std::string exprStr;
    bool valid = false;
    std::string lastError;
    NodePtr root;

    std::vector<Token> tokenize(const std::string& str);
    NodePtr parseExpression(const std::vector<Token>& tokens, size_t& pos);
    NodePtr parseTerm(const std::vector<Token>& tokens, size_t& pos);
    NodePtr parseFactor(const std::vector<Token>& tokens, size_t& pos);
    NodePtr parseUnary(const std::vector<Token>& tokens, size_t& pos);
    NodePtr parsePrimary(const std::vector<Token>& tokens, size_t& pos);
};
