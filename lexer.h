#ifndef CALK_LEXER_H
#define CALK_LEXER_H

#include <iostream>
#include <string>
#include <cctype>
#include <vector>

// Типы токенов
enum class TokenType {
    IDENTIFIER,  // идентификатор (a, x, _x, a_b_c)
    NUMBER,      // число (1, 1.0)
    OPERATOR,    // оператор (+, -, *, /, ^)
    LPAREN,      // левая скобка (
    RPAREN,      // правая скобка )
    EOEX,        // конец выражения
    ERROR        // лексическая ошибка
};


struct Token {
    TokenType type;
    std::string value;

    std::string view() const {
        return value;
    }
};

class Lexer {
private:
    const std::string& input;
    size_t pos;

    void skipWhitespace() {
        while (pos < input.size() && std::isspace(static_cast<unsigned char>(input[pos]))) {
            pos = pos + 1;
        }
    }

    void toLowerInPlace(std::string& s) {
        for (size_t i = 0; i < s.size(); i = i + 1) {
            if (s[i] >= 'A' && s[i] <= 'Z') {
                s[i] = s[i] + 32;
            }
        }
    }

    bool isIdentifierChar(char c) {
        return std::isalpha(static_cast<unsigned char>(c)) ||
               std::isdigit(static_cast<unsigned char>(c)) ||
               c == '_';
    }

    bool isIdentifierStart(char c) {
        return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
    }

    Token readIdentifier() {
        std::string value;
        while (pos < input.size() && isIdentifierChar(input[pos])) {
            value += input[pos];
            pos = pos + 1;
        }
        toLowerInPlace(value);
        return {TokenType::IDENTIFIER, value};
    }

    Token readNumber() {
        std::string value;
        bool hasDot = false;

        while (pos < input.size()) {
            char c = input[pos];
            if (std::isdigit(static_cast<unsigned char>(c))) {
                value += c;
                pos = pos + 1;
            } else if (c == '.' && !hasDot) {
                hasDot = true;
                value += c;
                pos = pos + 1;
                if (pos >= input.size() || !std::isdigit(static_cast<unsigned char>(input[pos]))) {
                    return {TokenType::ERROR, "ERROR"};
                }
            } else if (isIdentifierChar(c) && c != '.') {
                return {TokenType::ERROR, "ERROR"};
            } else {
                break;
            }
        }

        if (value.size() > 1 && value[0] == '0') {
            if (!hasDot || value[1] != '.') {
                return {TokenType::ERROR, "ERROR"};
            }
        }

        return {TokenType::NUMBER, value};
    }

public:
    Lexer(const std::string& expr) : input(expr), pos(0) {}

    Token next() {
        skipWhitespace();

        if (pos >= input.size()) {
            return {TokenType::EOEX, ""};
        }

        char c = input[pos];

        if (isIdentifierStart(c)) {
            return readIdentifier();
        }

        if (std::isdigit(static_cast<unsigned char>(c))) {
            return readNumber();
        }

        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^') {
            std::string val(1, c);
            pos = pos + 1;
            return {TokenType::OPERATOR, val};
        }
        if (c == '(') {
            pos = pos + 1;
            return {TokenType::LPAREN, "("};
        }
        if (c == ')') {
            pos = pos + 1;
            return {TokenType::RPAREN, ")"};
        }

        return {TokenType::ERROR, "ERROR"};
    }
};

std::string lexer_flow_concatenator(const std::string& expression) {
    Lexer lexer(expression);
    std::string result = "";
    Token tok = lexer.next();
    while (tok.type != TokenType::EOEX) {
        if (tok.type == TokenType::ERROR) {
            return "ERROR\n";
        }
        result += (tok.view() + "\n");
        tok = lexer.next();
    }
    return result;
}


#endif //CALK_LEXER_H