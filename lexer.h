#pragma once
#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <algorithm>


enum class TokenType                // ура типы токенов в лексере
{
    NUM,                            // число (1, 1.0) — для узла числа
    VAR,                            // идентификатор (x, my_var) — для узла переменной
    OP,                             // оператор (+, -, *, /, ^) — для узла бинарной или унарной операции
    FUNC,                           // имя функции (sin, log, sqrt) — для узла функции
    LPAREN,                         // левая скобка (
    RPAREN,                         // правая скобка )
    END,                            // конец выражения
    ERROR                           // лексическая ошибка
};

struct Token                        // структура токена
{
    TokenType type;                 // тип токена
    std::string value;              // строка значений
};

class Lexer
{
private:
    const std::string& input;       // строка входа
    size_t pos;                     // позиция в строке

    // если идентификатор совпадает, это FUNC, а не VAR
    static bool isFunctionName(const std::string& s)
    {
        return s == "sin" || s == "cos" || s == "tan" || s == "asin" || s == "acos" || s == "atan" || s == "exp" || s == "log" || s == "sqrt";
    }

    // пропуск пробелов
    void skipWhitespace()
    {
        while (pos < input.size() && std::isspace(static_cast<unsigned char>(input[pos])))
        {
            ++pos;
        }
    }

    // X -> x
    void toLowerInPlace(std::string& s)
    {
        for (char& c : s)
        {
            if (c >= 'A' && c <= 'Z') c = c + 32;
        }
    }

    // знак идентификатора - буква, нижнее подчеркивание или цифра
    bool isIdentifierChar(char c)
    {
        return std::isalpha(static_cast<unsigned char>(c)) || std::isdigit(static_cast<unsigned char>(c)) || c == '_';
    }

    // знак начала идентификатора - буква или нижнее подчеркивание
    bool isIdentifierStart(char c)
    {
        return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
    }

    // читаем идентификатор
    Token readIdentifier()
    {
        std::string value;
        while (pos < input.size() && isIdentifierChar(input[pos]))                 // пока позиция не конец строки или это знак идентификатора
        {
            value += input[pos];
            ++pos;
        }
        toLowerInPlace(value);                                                   // понижаем регистр
        // Если имя — функция, возвращаем FUNC, иначе VAR
        TokenType type = isFunctionName(value) ? TokenType::FUNC : TokenType::VAR;  // имя функции - функ, не имя - вар
        return {type, value};
    }

    // читаем число, базовая функция хз что пояснять
    Token readNumber()
    {
        std::string value;
        bool hasDot = false;

        while (pos < input.size())
        {
            char c = input[pos];
            if (std::isdigit(static_cast<unsigned char>(c)))
            {
                value += c;
                ++pos;
            } else if (c == '.' && !hasDot)
            {
                hasDot = true;
                value += c;
                ++pos;
                if (pos >= input.size() || !std::isdigit(static_cast<unsigned char>(input[pos])))
                {
                    return {TokenType::ERROR, "ERROR"};
                }
            } else if (isIdentifierChar(c) && c != '.')
            {
                return {TokenType::ERROR, "ERROR"}; // число сразу за буквой — ошибка
            } else
            {
                break;
            }
        }

        // проверка на ведущие нули, 002 - ошибка, 0.5 - ок
        if (value.size() > 1 && value[0] == '0' && value[1] != '.')
        {
            return {TokenType::ERROR, "ERROR"};
        }

        return {TokenType::NUM, value};
    }

public:
    Lexer(const std::string& expr) : input(expr), pos(0) {}

    // функция чтения следующего токена
    Token next()
    {
        skipWhitespace();                               // скип пробелов

        if (pos >= input.size())                        // вышли за пределы - ендим
        {
            return {TokenType::END, ""};
        }

        char c = input[pos];

        // в зависимости от условий либо вызываем функции, либо сразу записываем токен. Если ничего не выбрали - это будет ошибочка
        if (isIdentifierStart(c))
        {
            return readIdentifier();
        }
        if (std::isdigit(static_cast<unsigned char>(c)))
        {
            return readNumber();
        }
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^')
        {
            std::string val(1, c);
            ++pos;
            return {TokenType::OP, val};
        }
        if (c == '(')
        {
            ++pos;
            return {TokenType::LPAREN, "("};
        }
        if (c == ')')
        {
            ++pos;
            return {TokenType::RPAREN, ")"};
        }
        return {TokenType::ERROR, "ERROR"};
    }
};
