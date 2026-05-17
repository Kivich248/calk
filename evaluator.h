#pragma once
#include <cmath>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include "tree.h"


// вычисляет значение выражения по дереву.
// vars: карта переменных {"x": 3.0, "y": -1.5}
// функция представляет из себя большой свитч по типам узла, которая будет рекурсивно себя вызывать в нужных моментах
// передаем корень дерева + карту
double evaluate(const NodePtr& node, const std::unordered_map<std::string, double>& vars)
{
    switch (node->type)
    {
        case Node::NUM:
            return node->num;

        case Node::VAR:
        {
            auto it = vars.find(node->name);
            if (it == vars.end())
            {
                throw std::runtime_error("ERROR Unknown variable: " + node->name);
            }
            return it->second;          // если я правильно помню it->second дает обратиться ко "второй строке" этой как ее хэш-таблицы
        }

        case Node::BIN_OP:
        {
            double l = evaluate(node->left, vars);              // рекурсивно вызываем относительно детей
            double r = evaluate(node->right, vars);
            switch (node->op)
            {
                case '+': return l + r;
                case '-': return l - r;
                case '*': return l * r;
                case '/':
                    if (std::abs(r) < 1e-14) throw std::runtime_error("ERROR Domain error: division by zero");      // мы работаем с дабл, надо выкобениваться в сравнениях
                    return l / r;
                case '^': return std::pow(l, r);
                default: throw std::runtime_error("ERROR Syntax error: unknown binary operator");
            }
        }

        case Node::UNARY_OP:
        {
            double v = evaluate(node->arg, vars);
            return (node->op == '-') ? -v : v; // унарный + не меняет знак
        }

        case Node::FUNC:
        {
            double x = evaluate(node->arg, vars);
            auto domain_err = [&](const std::string& func)              // универсально каидем ошибки для красивой записи следующих ифов
            {
                throw std::runtime_error("ERROR Domain error: " + func + "(" + std::to_string(x) + ")");
            };

            if (node->name == "sin") return std::sin(x);
            if (node->name == "cos") return std::cos(x);
            if (node->name == "tan") return std::tan(x);
            if (node->name == "asin") { if (x < -1.0 || x > 1.0) domain_err(node->name); return std::asin(x); }
            if (node->name == "acos") { if (x < -1.0 || x > 1.0) domain_err(node->name); return std::acos(x); }
            if (node->name == "atan") return std::atan(x);
            if (node->name == "exp") return std::exp(x);
            if (node->name == "log")  { if (x <= 0.0) domain_err(node->name); return std::log(x); }
            if (node->name == "sqrt"){ if (x < 0.0) domain_err(node->name); return std::sqrt(x); }
            throw std::runtime_error("ERROR Unknown function: " + node->name);
        }
    }
    throw std::runtime_error("ERROR Syntax error: invalid node type");
}