#pragma once
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include "lexer.h"
#include "tree.h"

class Parser
{
private:
    std::vector<Token> tokens;                                                              // вектор токенов из лексера
    size_t pos = 0;                                                                         // позиция в векторе токенов

    // либо возвращаем токен номера pos, либо ошибка
    const Token& peek() const
    {
        if (pos >= tokens.size()) throw std::runtime_error("ERROR Syntax error");
        return tokens[pos];
    }

    // ожидаемый ли тип токена? + сдвиг поз
    bool match(TokenType type)
    {
        if (pos < tokens.size() && tokens[pos].type == type)
        {
            ++pos;
            return true;
        }
        return false;
    }

    // функции возвращающие ноудптр
    NodePtr parseExpr();
    NodePtr parseTerm();
    NodePtr parseUnary();
    NodePtr parsePower();
    NodePtr parsePrimary();

public:
    explicit Parser(std::vector<Token> t) : tokens(std::move(t)) {}             // явный конструктор класса

    // сам парсер
    NodePtr parse()
    {
        NodePtr root = parseExpr();
        if (pos != tokens.size())
        {
            throw std::runtime_error("ERROR Syntax error");
        }
        return root;
    }
};

// сложение и вычитание, низший приоритет
NodePtr Parser::parseExpr()
{
    NodePtr left = parseTerm();
    // явная рекурсия между функциями в порядке приоритета, по логике она создаст первую самую левую часть дерева

    while (pos < tokens.size() && tokens[pos].type == TokenType::OP)
    {
        std::string& opStr = tokens[pos].value;
        if (opStr == "+" || opStr == "-")
        {
            char op = opStr[0];
            ++pos;
            left = std::make_shared<Node>(op, std::move(left), parseTerm());
            // шоооок parseTerm() тут выступает как правый потомок, то есть рекурсия
            // make_shared<Node> делает умные штуки с памятью в куче и создает NodePtr через конструкторы Node
            // то есть мы уже построили все что можно в left в строке 58, вызвав рекурсию много раз и сменив pos, поэтому новый
            // parseTerm() выдаст не дерево left, тк глобальная переменная pos уже другая
            // и мы не пишем std::make_shared<Node>(op, parseTerm(), parseTerm()); поскольку это нарушит рекурсию и не гарантирует однозначность
            // когда мы делаем тут std::move(left) мы типо говорим что теперь то что было в left станет левым потомком нового узла left
            // типо вот было left, а станет вот такое left и потом начнет вычисляться уже правый parseTerm и менять pos
            //          *                           +
            //         / \                         / \
            //       sin  5                       *   parseTerm()
            //        |                          / \
            //        2                        sin  5
            //                                  |
            //                                  2
            // while тут работает не явно тк pos - глобальная переменная и по сути ты вот досчитал дерево, которое я написал выше и
            // while проверяется еще раз и может оказаться что новый pos снова подходит и дерево выше может снова сделать move(left)
            //                    -
            //                   / \
            //                  +   parseTerm() - новый, относительно нового pos
            //                 / \
            //                *   parseTerm()
            //               / \    (уже посчитан)
            //             sin  5    (и он изменил pos)
            //              |
            //              2
        } else break;
    }
    return left;
}

// умножение и деление, абсолютно аналогично, но вместо рекурсии в parseTerm, рекурсия в parseUnary
NodePtr Parser::parseTerm()
{
    NodePtr left = parseUnary();
    while (pos < tokens.size() && tokens[pos].type == TokenType::OP)
    {
        std::string& opStr = tokens[pos].value;
        if (opStr == "*" || opStr == "/")
        {
            char op = opStr[0];
            ++pos;
            left = std::make_shared<Node>(op, std::move(left), parseUnary());
        } else break;
    }
    return left;
}

// унарные + и -, строго один знак, приоритет выше чем у */÷, но ниже чем у ^
NodePtr Parser::parseUnary()
{
    if (pos < tokens.size() && tokens[pos].type == TokenType::OP)
    {
        std::string& opStr = tokens[pos].value;
        if (opStr == "+" || opStr == "-")
        {
            char op = opStr[0];
            ++pos;
            NodePtr operand = parsePower();     // рекурсия, считает правильно -2^2 = -4
            if (op == '+') return operand;      // либо вернули чисто часть дерева, либо узел с минусом
            return std::make_shared<Node>(op, std::move(operand));
        }
    }
    return parsePower();
}

// степень, ассоциативность правая
NodePtr Parser::parsePower()
{
    NodePtr left = parsePrimary();
    if (pos < tokens.size() && tokens[pos].type == TokenType::OP && tokens[pos].value == "^")     // все три условия степени соблюдены
    {
        ++pos;
        left = std::make_shared<Node>('^', std::move(left), parseUnary());                  // парсе унари дает возможность существовать 2^(-2)
    }
    return left;
}

// числа, переменные, функции, скобки
NodePtr Parser::parsePrimary()
{
    if (match(TokenType::NUM))
    {
        return std::make_shared<Node>(std::stod(tokens[pos-1].value));      // -1 тк код функции match делает pos ++
    }
    if (match(TokenType::VAR))
    {
        return std::make_shared<Node>(tokens[pos-1].value);
    }
    if (match(TokenType::FUNC))         // если вокруг аргумента функции нет скобок - ггвп, матч используем свободно за счет ++ в теле функции
    {
        std::string fname = tokens[pos-1].value;
        if (!match(TokenType::LPAREN)) throw std::runtime_error("ERROR Syntax error");
        NodePtr arg = parseExpr();                                                      // рекурсия с нуля внутри скобок
        if (!match(TokenType::RPAREN)) throw std::runtime_error("ERROR Syntax error");
        return std::make_shared<Node>(fname, std::move(arg));
    }
    if (match(TokenType::LPAREN))                                                       // рекурсия с нуля внутри скобок
    {
        NodePtr expr = parseExpr();
        if (!match(TokenType::RPAREN)) throw std::runtime_error("ERROR Syntax error");
        return expr;
    }
    throw std::runtime_error("ERROR Syntax error");
}


// теоретически, дополнить виды скобок тут достаточно просто, я бы добавил функцию сравнения типа скобок через номер в ASCII. То есть
// перед match я бы записал номер ASCII в отдельные uint_8
// к сожалению, номера скобок не симметричны, поэтому пришлось бы ручками работать/минифункцию накатать и добавить в иф помимо
// мэтча с правильным типом в целом

// upd от 09.06: я честно хотел это сделать, но спустя 4 дня после того как закончил лабу я заболел сначала ангиной, потом пневмонией
// и вот только вышел с больничного
