#pragma once
#include <memory>
#include <cmath>
#include "tree.h"

// рекурсивное упрощение дерева выражений.

NodePtr simplify(NodePtr node) {
    if (!node) return nullptr;

    // сначала рекурсивно упрощаем дочерние узлы - это гарантирует, что правила применятся к вложенным выражениям снизу вверх.
    switch (node->type)
    {
        case Node::BIN_OP:
            node->left = simplify(node->left);
            node->right = simplify(node->right);
            break;
        case Node::UNARY_OP:
            node->arg = simplify(node->arg);
            break;
        case Node::FUNC:
            node->arg = simplify(node->arg);
            break;
        default: break;
    }

    // применяем правила упрощения к текущему узлу
    switch (node->type)
    {
        case Node::NUM:
        case Node::VAR:
            return node;

        case Node::UNARY_OP:
        {
            // +x -> x
            if (node->op == '+') return node->arg;

            // -(-x) -> x
            if (node->op == '-' && node->arg &&
                node->arg->type == Node::UNARY_OP && node->arg->op == '-') {
                return node->arg->arg;
            }
            break;
        }

        case Node::BIN_OP:
        {
            bool l_is_num = (node->left && node->left->type == Node::NUM);
            bool r_is_num = (node->right && node->right->type == Node::NUM);
            double l_val = l_is_num ? node->left->num : 0.0;
            double r_val = r_is_num ? node->right->num : 0.0;
            // долго упорно страдаем чутка упрощая все
            switch (node->op)
            {
                case '+':
                    if (l_is_num && l_val == 0.0) return node->right; // 0 + x
                    if (r_is_num && r_val == 0.0) return node->left;  // x + 0
                    if (l_is_num && r_is_num) return std::make_shared<Node>(l_val + r_val);
                    break;
                case '-':
                    if (r_is_num && r_val == 0.0) return node->left;  // x - 0
                    if (l_is_num && l_val == 0.0)                     // 0 - x -> -x
                        return std::make_shared<Node>('-', node->right);
                    if (l_is_num && r_is_num) return std::make_shared<Node>(l_val - r_val);
                    break;
                case '*':
                    if ((l_is_num && l_val == 0.0) || (r_is_num && r_val == 0.0))
                        return std::make_shared<Node>(0.0);
                    if (l_is_num && l_val == 1.0) return node->right;
                    if (r_is_num && r_val == 1.0) return node->left;
                    if (l_is_num && r_is_num) return std::make_shared<Node>(l_val * r_val);
                    break;
                case '/':
                    if (r_is_num && r_val == 1.0) return node->left;
                    if (l_is_num && r_is_num) return std::make_shared<Node>(l_val / r_val);
                    break;
                case '^':
                    if (r_is_num && r_val == 0.0) return std::make_shared<Node>(1.0);
                    if (r_is_num && r_val == 1.0) return node->left;
                    if (l_is_num && l_val == 1.0) return std::make_shared<Node>(1.0);
                    if (l_is_num && r_is_num) return std::make_shared<Node>(std::pow(l_val, r_val));
                    break;
            }
            break;
        }
    }
    return node;
}