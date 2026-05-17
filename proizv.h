#pragma once
#include <memory>
#include <string>
#include "tree.h"

inline NodePtr cloneNode(const NodePtr& n)
{
    return n ? std::make_shared<Node>(*n) : nullptr;
}


// по сути все примерно так же как овуляторе, но чутка сложнее. суть та же, математика изменилась
NodePtr derivative(const NodePtr& node, const std::string& var)
{
    switch (node->type)
    {
        // const` = 0
        case Node::NUM:
            return std::make_shared<Node>(0.0);

        // x` = 1, y` = 0
        case Node::VAR:
            return std::make_shared<Node>(node->name == var ? 1.0 : 0.0);

        // (-u)` = -u`
        case Node::UNARY_OP:
        {
            NodePtr d_arg = derivative(node->arg, var);
            if (node->op == '-') return std::make_shared<Node>('-', d_arg);
            return d_arg; // унарный + не меняет знак
        }

        // (u + v)` = u` + v`
        // (u - v)` = u` - v`
        // (u * v)` = u` * v + u * v`
        // (u / v)` = ( u` * v - u * v`) / v^2
        // (u ^ c)` = c * u` * u ^ (c - 1)
        // (u ^ v)` = u^v * (v` * ln(u) + v * (u` / u))
        case Node::BIN_OP:
        {
            NodePtr du = derivative(node->left, var);
            NodePtr dv = derivative(node->right, var);
            NodePtr u = cloneNode(node->left);
            NodePtr v = cloneNode(node->right);

            switch (node->op)
            {
                case '+': return std::make_shared<Node>('+', du, dv);
                case '-': return std::make_shared<Node>('-', du, dv);
                case '*':
                    return std::make_shared<Node>('+',
                        std::make_shared<Node>('*', du, v),
                        std::make_shared<Node>('*', u, dv)
                    );
                case '/':
                    return std::make_shared<Node>('/',
                        std::make_shared<Node>('-',
                            std::make_shared<Node>('*', du, v),
                            std::make_shared<Node>('*', u, dv)
                        ),
                        std::make_shared<Node>('^', v, std::make_shared<Node>(2.0))
                    );
                case '^':
                {
                    if (node->right->type == Node::NUM) {
                        double c = node->right->num;
                        if (c == 0.0) return std::make_shared<Node>(0.0);
                        if (c == 1.0) return du; // производная от u^1 равна u'

                        NodePtr u_pow = std::make_shared<Node>('^', cloneNode(node->left), std::make_shared<Node>(c - 1.0));
                        return std::make_shared<Node>('*', std::make_shared<Node>(c), std::make_shared<Node>('*', u_pow, du));
                    }

                    NodePtr ln_u = std::make_shared<Node>("log", cloneNode(node->left));
                    NodePtr term1 = std::make_shared<Node>('*', dv, ln_u);
                    NodePtr term2 = std::make_shared<Node>('*', cloneNode(node->right), std::make_shared<Node>('/', du, cloneNode(node->left)));
                    NodePtr inner = std::make_shared<Node>('+', term1, term2);
                    return std::make_shared<Node>('*', std::make_shared<Node>('^', cloneNode(node->left), cloneNode(node->right)), inner);
                }
                default: return std::make_shared<Node>(0.0);
            }
        }

        // мне слишком лень прописывать производной каждой функции, мы блин на математике учимся, это очев
        case Node::FUNC:
        {
            NodePtr du = derivative(node->arg, var);
            NodePtr arg = cloneNode(node->arg);
            NodePtr f_prime;

            if (node->name == "sin")
            {
                f_prime = std::make_shared<Node>("cos", arg);
            } else if (node->name == "cos")
            {
                f_prime = std::make_shared<Node>('-', std::make_shared<Node>("sin", arg));
            } else if (node->name == "tan")
            {
                NodePtr cos_u = std::make_shared<Node>("cos", cloneNode(arg));
                f_prime = std::make_shared<Node>('/', std::make_shared<Node>(1.0), std::make_shared<Node>('^', cos_u, std::make_shared<Node>(2.0)));
            } else if (node->name == "asin")
            {
                NodePtr u2 = std::make_shared<Node>('^', cloneNode(arg), std::make_shared<Node>(2.0));
                NodePtr inner = std::make_shared<Node>('-', std::make_shared<Node>(1.0), u2);
                f_prime = std::make_shared<Node>('/', std::make_shared<Node>(1.0), std::make_shared<Node>("sqrt", inner));
            } else if (node->name == "acos")
            {
                NodePtr u2 = std::make_shared<Node>('^', cloneNode(arg), std::make_shared<Node>(2.0));
                NodePtr inner = std::make_shared<Node>('-', std::make_shared<Node>(1.0), u2);
                f_prime = std::make_shared<Node>('-', std::make_shared<Node>('/', std::make_shared<Node>(1.0), std::make_shared<Node>("sqrt", inner)));
            } else if (node->name == "atan")
            {
                NodePtr u2 = std::make_shared<Node>('^', cloneNode(arg), std::make_shared<Node>(2.0));
                f_prime = std::make_shared<Node>('/', std::make_shared<Node>(1.0), std::make_shared<Node>('+', std::make_shared<Node>(1.0), u2));
            } else if (node->name == "exp")
            {
                f_prime = std::make_shared<Node>("exp", arg);
            } else if (node->name == "log")
            {
                f_prime = std::make_shared<Node>('/', std::make_shared<Node>(1.0), arg);
            } else if (node->name == "sqrt")
            {
                f_prime = std::make_shared<Node>('/', std::make_shared<Node>(1.0), std::make_shared<Node>('*', std::make_shared<Node>(2.0), std::make_shared<Node>("sqrt", arg)));
            } else
            {
                f_prime = std::make_shared<Node>(0.0);
            }

            return std::make_shared<Node>('*', f_prime, du);
        }
    }
    return nullptr;
}