#include "MathParser.hpp"
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <functional>

namespace {
    struct NumberNode : public MathParser::Node {
        double val;
        explicit NumberNode(double v) : val(v) {}
        double evaluate(double) const override { return val; }
    };

    struct VariableNode : public MathParser::Node {
        double evaluate(double t) const override { return t; }
    };

    struct BinaryOpNode : public MathParser::Node {
        char op;
        MathParser::NodePtr left, right;
        BinaryOpNode(char o, MathParser::NodePtr l, MathParser::NodePtr r)
            : op(o), left(std::move(l)), right(std::move(r)) {}
        double evaluate(double t) const override {
            double l = left ? left->evaluate(t) : 0.0;
            double r = right ? right->evaluate(t) : 0.0;
            switch (op) {
                case '+': return l + r;
                case '-': return l - r;
                case '*': return l * r;
                case '/': return (std::abs(r) > 1e-12) ? l / r : 0.0;
                case '^': return std::pow(l, r);
                default: return 0.0;
            }
        }
    };

    struct UnaryOpNode : public MathParser::Node {
        char op;
        MathParser::NodePtr child;
        UnaryOpNode(char o, MathParser::NodePtr c) : op(o), child(std::move(c)) {}
        double evaluate(double t) const override {
            double v = child ? child->evaluate(t) : 0.0;
            return (op == '-') ? -v : v;
        }
    };

    struct FunctionNode : public MathParser::Node {
        std::string funcName;
        MathParser::NodePtr arg;
        FunctionNode(std::string name, MathParser::NodePtr a)
            : funcName(std::move(name)), arg(std::move(a)) {}
        double evaluate(double t) const override {
            double v = arg ? arg->evaluate(t) : 0.0;
            if (funcName == "sin") return std::sin(v);
            if (funcName == "cos") return std::cos(v);
            if (funcName == "tan") return std::tan(v);
            if (funcName == "asin") return (v >= -1.0 && v <= 1.0) ? std::asin(v) : 0.0;
            if (funcName == "acos") return (v >= -1.0 && v <= 1.0) ? std::acos(v) : 0.0;
            if (funcName == "atan") return std::atan(v);
            if (funcName == "sinh") return std::sinh(v);
            if (funcName == "cosh") return std::cosh(v);
            if (funcName == "tanh") return std::tanh(v);
            if (funcName == "exp") return std::exp(v);
            if (funcName == "log" || funcName == "ln") return (v > 1e-12) ? std::log(v) : 0.0;
            if (funcName == "sqrt") return (v >= 0.0) ? std::sqrt(v) : 0.0;
            if (funcName == "abs") return std::abs(v);
            if (funcName == "sgn" || funcName == "sign") return (v > 1e-7) ? 1.0 : ((v < -1e-7) ? -1.0 : 0.0);
            if (funcName == "step" || funcName == "heaviside") return (v >= 0.0) ? 1.0 : 0.0;
            if (funcName == "floor") return std::floor(v);
            if (funcName == "ceil") return std::ceil(v);
            return 0.0;
        }
    };
}

MathParser::MathParser() : valid(false) {
}

MathParser::MathParser(const std::string& expression) {
    setExpression(expression);
}

bool MathParser::setExpression(const std::string& expression) {
    exprStr = expression;
    lastError.clear();
    root = nullptr;
    valid = false;

    if (expression.empty()) {
        valid = true;
        root = std::make_shared<NumberNode>(0.0);
        return true;
    }

    try {
        auto tokens = tokenize(expression);
        if (tokens.empty() || (tokens.size() == 1 && tokens[0].type == TokenType::End)) {
            valid = true;
            root = std::make_shared<NumberNode>(0.0);
            return true;
        }
        size_t pos = 0;
        root = parseExpression(tokens, pos);
        if (pos < tokens.size() && tokens[pos].type != TokenType::End) {
            lastError = "Unexpected token after expression: " + tokens[pos].text;
            valid = false;
            return false;
        }
        valid = true;
        return true;
    }
    catch (const std::exception& ex) {
        lastError = ex.what();
        valid = false;
        root = nullptr;
        return false;
    }
}

double MathParser::evaluate(double t) const {
    if (!valid || !root) return 0.0;
    try {
        return root->evaluate(t);
    }
    catch (...) {
        return 0.0;
    }
}

std::vector<MathParser::Token> MathParser::tokenize(const std::string& str) {
    std::vector<Token> tokens;
    size_t i = 0;
    const size_t n = str.size();

    while (i < n) {
        char c = str[i];
        if (std::isspace(static_cast<unsigned char>(c))) {
            i++;
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
            size_t start = i;
            bool dotSeen = (c == '.');
            i++;
            while (i < n && (std::isdigit(static_cast<unsigned char>(str[i])) || (!dotSeen && str[i] == '.'))) {
                if (str[i] == '.') dotSeen = true;
                i++;
            }
            std::string numStr = str.substr(start, i - start);
            double v = std::stod(numStr);
            tokens.push_back({ TokenType::Number, v, numStr });
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            size_t start = i;
            while (i < n && (std::isalnum(static_cast<unsigned char>(str[i])) || str[i] == '_')) {
                i++;
            }
            std::string name = str.substr(start, i - start);
            for (auto& ch : name) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

            if (name == "t") {
                tokens.push_back({ TokenType::Variable, 0.0, name });
            }
            else if (name == "pi") {
                tokens.push_back({ TokenType::Number, 3.14159265358979323846, name });
            }
            else if (name == "e") {
                tokens.push_back({ TokenType::Number, 2.71828182845904523536, name });
            }
            else {
                tokens.push_back({ TokenType::Function, 0.0, name });
            }
            continue;
        }

        switch (c) {
            case '+': tokens.push_back({ TokenType::Plus, 0.0, "+" }); i++; break;
            case '-': tokens.push_back({ TokenType::Minus, 0.0, "-" }); i++; break;
            case '*': tokens.push_back({ TokenType::Multiply, 0.0, "*" }); i++; break;
            case '/': tokens.push_back({ TokenType::Divide, 0.0, "/" }); i++; break;
            case '^': tokens.push_back({ TokenType::Power, 0.0, "^" }); i++; break;
            case '(': tokens.push_back({ TokenType::LParen, 0.0, "(" }); i++; break;
            case ')': tokens.push_back({ TokenType::RParen, 0.0, ")" }); i++; break;
            case ',': tokens.push_back({ TokenType::Comma, 0.0, "," }); i++; break;
            default:
                throw std::runtime_error(std::string("Unknown character: '") + c + "'");
        }
    }
    tokens.push_back({ TokenType::End, 0.0, "" });
    return tokens;
}

MathParser::NodePtr MathParser::parseExpression(const std::vector<Token>& tokens, size_t& pos) {
    auto node = parseTerm(tokens, pos);
    while (pos < tokens.size() && (tokens[pos].type == TokenType::Plus || tokens[pos].type == TokenType::Minus)) {
        char op = (tokens[pos].type == TokenType::Plus) ? '+' : '-';
        pos++;
        auto right = parseTerm(tokens, pos);
        node = std::make_shared<BinaryOpNode>(op, node, right);
    }
    return node;
}

MathParser::NodePtr MathParser::parseTerm(const std::vector<Token>& tokens, size_t& pos) {
    auto node = parseFactor(tokens, pos);
    while (pos < tokens.size() && (tokens[pos].type == TokenType::Multiply || tokens[pos].type == TokenType::Divide)) {
        char op = (tokens[pos].type == TokenType::Multiply) ? '*' : '/';
        pos++;
        auto right = parseFactor(tokens, pos);
        node = std::make_shared<BinaryOpNode>(op, node, right);
    }
    return node;
}

MathParser::NodePtr MathParser::parseFactor(const std::vector<Token>& tokens, size_t& pos) {
    auto node = parseUnary(tokens, pos);
    if (pos < tokens.size() && tokens[pos].type == TokenType::Power) {
        pos++;
        auto right = parseFactor(tokens, pos); // right-associative
        node = std::make_shared<BinaryOpNode>('^', node, right);
    }
    return node;
}

MathParser::NodePtr MathParser::parseUnary(const std::vector<Token>& tokens, size_t& pos) {
    if (pos < tokens.size() && (tokens[pos].type == TokenType::Plus || tokens[pos].type == TokenType::Minus)) {
        char op = (tokens[pos].type == TokenType::Plus) ? '+' : '-';
        pos++;
        auto child = parseUnary(tokens, pos);
        return std::make_shared<UnaryOpNode>(op, child);
    }
    return parsePrimary(tokens, pos);
}

MathParser::NodePtr MathParser::parsePrimary(const std::vector<Token>& tokens, size_t& pos) {
    if (pos >= tokens.size()) {
        throw std::runtime_error("Unexpected end of expression");
    }

    const auto& tok = tokens[pos];
    if (tok.type == TokenType::Number) {
        pos++;
        return std::make_shared<NumberNode>(tok.value);
    }
    if (tok.type == TokenType::Variable) {
        pos++;
        return std::make_shared<VariableNode>();
    }
    if (tok.type == TokenType::Function) {
        std::string fname = tok.text;
        pos++;
        if (pos >= tokens.size() || tokens[pos].type != TokenType::LParen) {
            throw std::runtime_error("Expected '(' after function '" + fname + "'");
        }
        pos++; // skip '('
        auto arg = parseExpression(tokens, pos);
        if (pos >= tokens.size() || tokens[pos].type != TokenType::RParen) {
            throw std::runtime_error("Expected ')' closing function argument for '" + fname + "'");
        }
        pos++; // skip ')'
        return std::make_shared<FunctionNode>(fname, arg);
    }
    if (tok.type == TokenType::LParen) {
        pos++;
        auto node = parseExpression(tokens, pos);
        if (pos >= tokens.size() || tokens[pos].type != TokenType::RParen) {
            throw std::runtime_error("Expected closing ')'");
        }
        pos++;
        return node;
    }

    throw std::runtime_error("Unexpected token: " + tok.text);
}
