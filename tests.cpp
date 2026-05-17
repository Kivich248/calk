#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <functional>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

#include "lexer.h"
#include "tree.h"
#include "parser.h"
#include "evaluator.h"
#include "proizv.h"
#include "simple.h"


// приведения регистра
static std::string toLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

// запуск подсчтеа
double run_eval(const std::string& expr, const std::unordered_map<std::string, double>& vars = {}) {
    std::unordered_map<std::string, double> norm_vars;
    for (const auto& [k, v] : vars) norm_vars[toLower(k)] = v;

    Lexer lexer(expr);
    std::vector<Token> tokens;
    Token tok = lexer.next();
    while (tok.type != TokenType::END) {
        if (tok.type == TokenType::ERROR) throw std::runtime_error("ERROR Lexical error");
        tokens.push_back(tok);
        tok = lexer.next();
    }
    Parser parser(std::move(tokens));
    NodePtr tree = parser.parse();
    return evaluate(tree, norm_vars);
}

// запуск производной + подсчета
double run_deriv_eval(const std::string& expr, const std::string& var, double val) {
    Lexer lexer(expr);
    std::vector<Token> tokens;
    Token tok = lexer.next();
    while (tok.type != TokenType::END) {
        if (tok.type == TokenType::ERROR) throw std::runtime_error("ERROR Lexical error");
        tokens.push_back(tok);
        tok = lexer.next();
    }
    Parser parser(std::move(tokens));
    NodePtr tree = parser.parse();
    NodePtr der = simplify(derivative(tree, toLower(var)));
    return evaluate(der, {{toLower(var), val}});
}

// макросы проверок
// разница чисел превышает епс - плохо
#define EXPECT_NEAR(a, b, eps) \
    if (std::abs((a) - (b)) > (eps)) \
        throw std::runtime_error("Value mismatch: " #a " ~= " #b ", got " + std::to_string(a))

// проверяем выброс ошибки: выбросилоли, правильно ли написало
#define EXPECT_ERROR(expr, substr) \
    try { \
        expr; \
        throw std::runtime_error("Expected exception but none thrown for: " #expr); \
    } catch (const std::exception& e) { \
        std::string msg = e.what(); \
        if (msg.find("ERROR") == std::string::npos) \
            throw std::runtime_error("Error must start with ERROR. Got: " + msg); \
        if (msg.find(substr) == std::string::npos) \
            throw std::runtime_error("Wrong error: " + msg + " (expected '" substr "')"); \
    }

// я не очень понял как оно работает но оно работает
std::vector<std::pair<std::string, std::function<void()>>> g_tests;
void run(const std::string& name, std::function<void()> func) { g_tests.emplace_back(name, func); }
#define TEST(name) \
    void test_##name(); \
    struct TestReg_##name { TestReg_##name() { run(#name, test_##name); } } reg_##name; \
    void test_##name()

// что мы получили сравниваем с тем что должны получить с допустимой погрешностью по критериям
TEST(arithmetic_pos_neg_zero)
{
    // +
    EXPECT_NEAR(run_eval("5 + 3"), 8.0, 1e-6);
    EXPECT_NEAR(run_eval("5 + (-3)"), 2.0, 1e-6);
    EXPECT_NEAR(run_eval("-5 + 3"), -2.0, 1e-6);
    EXPECT_NEAR(run_eval("0 + 0"), 0.0, 1e-6);
    
    // -
    EXPECT_NEAR(run_eval("5 - 3"), 2.0, 1e-6);
    EXPECT_NEAR(run_eval("3 - 5"), -2.0, 1e-6);
    EXPECT_NEAR(run_eval("-5 - (-3)"), -2.0, 1e-6);
    EXPECT_NEAR(run_eval("0 - 5"), -5.0, 1e-6);
    
    // *
    EXPECT_NEAR(run_eval("4 * 3"), 12.0, 1e-6);
    EXPECT_NEAR(run_eval("-4 * 3"), -12.0, 1e-6);
    EXPECT_NEAR(run_eval("0 * 100"), 0.0, 1e-6);
    EXPECT_NEAR(run_eval("-2 * -3"), 6.0, 1e-6);
    
    // /
    EXPECT_NEAR(run_eval("10 / 2"), 5.0, 1e-6);
    EXPECT_NEAR(run_eval("-10 / 2"), -5.0, 1e-6);
    EXPECT_NEAR(run_eval("0 / 5"), 0.0, 1e-6);
    EXPECT_NEAR(run_eval("1 / -4"), -0.25, 1e-6);
    
    // ^
    EXPECT_NEAR(run_eval("-2^2"), -4.0, 1e-6);      // унарный минус ниже ^
    EXPECT_NEAR(run_eval("(-2)^2"), 4.0, 1e-6);     // скобки меняют приоритет
    EXPECT_NEAR(run_eval("2^3^2"), 512.0, 1e-6);    // правоассоциативность
}

// посложнее
TEST(complex_evaluate_10_cases) {
    EXPECT_NEAR(run_eval("((2 + 3) * (4 - 1)) / 5"), 3.0, 1e-6);
    EXPECT_NEAR(run_eval("sqrt(3^2 + 4^2)"), 5.0, 1e-6);
    EXPECT_NEAR(run_eval("sin(0) + cos(0) + exp(0)"), 2.0, 1e-6);
    EXPECT_NEAR(run_eval("log(exp(5))"), 5.0, 1e-5);
    EXPECT_NEAR(run_eval("2^-2 * 4"), 1.0, 1e-6);
    EXPECT_NEAR(run_eval("-(x^2) + 2*x - 1", {{"x", 2.0}}), -1.0, 1e-6);
    EXPECT_NEAR(run_eval("tan(atan(1))"), 1.0, 1e-5);
    EXPECT_NEAR(run_eval("sqrt(16) / 2 + log(exp(1))"), 3.0, 1e-6);
    EXPECT_NEAR(run_eval("x*y + z", {{"x", 1.0}, {"y", 2.0}, {"z", 3.0}}), 5.0, 1e-6);
    EXPECT_NEAR(run_eval("sin(x)^2 + cos(x)^2", {{"x", 0.7}}), 1.0, 1e-5);
}

// тк производные выходят в странном виде, я проверяю 100 различных точек, что дает некоторые гарантии правильности
TEST(derivatives_100_points) {
    // f(x) = sin(x)*exp(x) -> f'(x) = exp(x)*(sin(x) + cos(x))
    for (int i = 0; i < 100; ++i)
    {
        double x = -2.0 + i * 0.04;
        double expected = std::exp(x) * (std::sin(x) + std::cos(x));
        double got = run_deriv_eval("sin(x)*exp(x)", "x", x);
        if (std::abs(got - expected) > 1e-5)
            throw std::runtime_error("Deriv sin*exp failed at x=" + std::to_string(x));
    }

    // f(x) = log(x^2 + 1) -> f'(x) = 2x / (x^2 + 1)
    for (int i = 0; i < 100; ++i) {
        double x = -3.0 + i * 0.06;
        double expected = (2.0 * x) / (x*x + 1.0);
        double got = run_deriv_eval("log(x^2 + 1)", "x", x);
        if (std::abs(got - expected) > 1e-5)
            throw std::runtime_error("Deriv log(x^2+1) failed at x=" + std::to_string(x));
    }

    // f(x) = atan(x) + x^3 -> f'(x) = 1/(1+x^2) + 3x^2
    for (int i = 0; i < 100; ++i) {
        double x = -2.5 + i * 0.05;
        double expected = 1.0 / (1.0 + x*x) + 3.0 * x * x;
        double got = run_deriv_eval("atan(x) + x^3", "x", x);
        if (std::abs(got - expected) > 1e-4)
            throw std::runtime_error("Deriv atan+x^3 failed at x=" + std::to_string(x));
    }
}

// области опр и ошибки
TEST(domain_boundaries)
{
    // sqrt: x < 0 -> error, x = 0 -> ok
    EXPECT_NEAR(run_eval("sqrt(0)"), 0.0, 1e-6);
    EXPECT_ERROR(run_eval("sqrt(-0.001)"), "Domain error");
    
    // log: x <= 0 -> error
    EXPECT_ERROR(run_eval("log(0)"), "Domain error");
    EXPECT_ERROR(run_eval("log(-2)"), "Domain error");
    EXPECT_NEAR(run_eval("log(exp(1))"), 1.0, 1e-5);
    
    // asin/acos: |x| > 1 -> error, |x| == 1 -> ok
    EXPECT_NEAR(run_eval("asin(1.0)"), std::asin(1.0), 1e-6);
    EXPECT_NEAR(run_eval("acos(-1.0)"), std::acos(-1.0), 1e-6);
    EXPECT_ERROR(run_eval("asin(1.0001)"), "Domain error");
    EXPECT_ERROR(run_eval("acos(-1.5)"), "Domain error");
    
    // деление на ноль -> плохо
    EXPECT_ERROR(run_eval("1/0"), "Domain error");
    EXPECT_ERROR(run_eval("5/(1-1)"), "Domain error");
}

// синтаксис
TEST(invalid_inputs_and_syntax)
{
    EXPECT_NEAR(run_eval("2++3"), 5.0, 1e-6);
    EXPECT_NEAR(run_eval("2--3"), 5.0, 1e-6);

    EXPECT_ERROR(run_eval("sin("), "Syntax error");
    EXPECT_ERROR(run_eval("x^"), "Syntax error");
    EXPECT_ERROR(run_eval("(2+3"), "Syntax error");
    EXPECT_ERROR(run_eval(")2+3"), "Syntax error");

    EXPECT_ERROR(run_eval("007"), "Lexical error");
    EXPECT_ERROR(run_eval("002.5"), "Lexical error");
    EXPECT_ERROR(run_eval("2.5.3"), "Lexical error");

    EXPECT_ERROR(run_eval("y + 1", {{"x", 5.0}}), "Unknown variable");

    EXPECT_ERROR(run_eval(""), "Syntax error");
}

// проверяем, что парсер/эвалюатор устойчив к пустым выражениям
TEST(io_format_simulation)
{
    EXPECT_ERROR(run_eval(""), "Syntax error");
}

// консольный запуск
// g++ -std=c++17 -O2 tests.cpp -o run_tests
// ./run_tests

int main()
{
    std::cout << "Running " << g_tests.size() << " test suites...\n\n";
    int passed = 0, failed = 0;

    // типо все тесты выше это какой-то массив, тот макрос позволяет писать тесты так просто, мы по ним идем, пробуем
    // если все ок пишет ок
    // не ок пишет не ок
    for (auto& [name, func] : g_tests)
    {
        try {
            func();
            std::cout << "[OK]    " << name << "\n";
            ++passed;
        } catch (const std::exception& e) {
            std::cout << "[FAIL]  " << name << "\n        Reason: " << e.what() << "\n";
            ++failed;
        }
    }

    std::cout << "Results: " << passed << " passed, " << failed << " failed\n";
    return failed > 0 ? 1 : 0;
}