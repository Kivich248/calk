#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <limits>
#include <stdexcept>

#include "lexer.h"
#include "tree.h"
#include "parser.h"
#include "evaluator.h"
#include "proizv.h"
#include "simple.h"

// безопасное приведение к нижнему регистру
static std::string toLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c)
    {
        return std::tolower(c);
    });
    return s;
}

// удаление символов \r для кроссплатформенной совместимости
static std::string cleanLine(std::string s)
{
    s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
    return s;
}

int main()
{
    // что-то для быстрого и корректного ввода
    std::ios_base::sync_with_stdio(false);
    std::cout << std::setprecision(10) << std::defaultfloat << std::unitbuf;

    std::string command;
    int n;

    // читаем пары "команда n" до конца
    while (std::cin >> command >> n)
    {
        try {
            // читаем имена переменных
            std::vector<std::string> varNames(n);
            for (int i = 0; i < n; ++i)
            {
                std::cin >> varNames[i];
            }

            // читаем значения
            std::unordered_map<std::string, double> vars;
            for (int i = 0; i < n; ++i)
            {
                double val;
                std::cin >> val;
                vars[toLower(varNames[i])] = val;
            }

            // без этого вывод не работал до того момента, как я начну новую итерацию цикла
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            // читаем строку с выражением
            std::string expression;
            std::getline(std::cin, expression);
            expression = cleanLine(expression);

            if (expression.empty())
            {
                throw std::runtime_error("ERROR Syntax error: empty expression");
            }

            // страдания. применяем лексер, массив токенов, заполняем массив токенов
            Lexer lexer(expression);
            std::vector<Token> tokens;
            Token tok = lexer.next();
            while (tok.type != TokenType::END) {
                if (tok.type == TokenType::ERROR) {
                    throw std::runtime_error("ERROR Lexical error");
                }
                tokens.push_back(tok);
                tok = lexer.next();
            }

            // кажется все просто, а я страдал над тем чтобы эти две строчки правда строили дерево сутки-другие
            // конструктим parser и применяем к нему метода parse
            Parser parser(std::move(tokens));
            NodePtr tree = parser.parse();

            // выбор команды и ее выполнение
            if (command == "evaluate")
            {
                std::cout << evaluate(tree, vars) << "\n";
            }
            else if (command == "derivative")
            {
                if (n == 0) throw std::runtime_error("ERROR Syntax error: no variables for derivative");
                std::string diffVar = toLower(varNames[0]);
                NodePtr derTree = simplify(derivative(tree, diffVar));
                std::cout << derTree->toString() << "\n";
            }
            else if (command == "evaluate_derivative")
            {
                if (n == 0) throw std::runtime_error("ERROR Syntax error: no variables for derivative");
                std::string diffVar = toLower(varNames[0]);
                NodePtr derTree = simplify(derivative(tree, diffVar));
                std::cout << evaluate(derTree, vars) << "\n";
            }
            else
            {
                throw std::runtime_error("ERROR Unknown command: " + command);
            }

        } catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }
    return 0;
}