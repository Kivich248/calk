#pragma once
#include <string>
#include <memory>
#include <cctype>
#include <algorithm>

struct Node;
using NodePtr = std::shared_ptr<Node>;

struct Node
{
	enum Type { NUM, VAR, BIN_OP, UNARY_OP, FUNC } type;													// тип узелка
	double num = 0;																							// если число - заменим
	std::string name;																						// имя переменной
	char op = 0;																							// знак операции
	NodePtr left;																							// правый и левый потомки для некоммутативных операций
	NodePtr right;
	NodePtr arg;																							// аргумент функции/унарный операции

	Node(double v) : type(NUM), num(v) {}																	// конструктор чиселка

	Node(std::string n) : type(VAR), name(std::move(n))														// конструктор переменной
	{
		for (char& c : name) c = std::tolower(static_cast<unsigned char>(c));
	}

	Node(char o, NodePtr l, NodePtr r) : type(BIN_OP), op(o), left(std::move(l)), right(std::move(r)) {}	// конструктор бинарной операции

	Node(char o, NodePtr a) : type(UNARY_OP), op(o), arg(std::move(a)) {}									// конструктор унарной операции

	Node(std::string n, NodePtr a) : type(FUNC), name(std::move(n)), arg(std::move(a))						// конструктор функции
	{
		for (char& c : name) c = std::tolower(static_cast<unsigned char>(c));
	}

	Node(const Node& other) : type(other.type), num(other.num), name(other.name), op(other.op)			// ура копирование
	{
		if (other.left)  left  = NodePtr(new Node(*other.left));
		if (other.right) right = NodePtr(new Node(*other.right));
		if (other.arg)   arg   = NodePtr(new Node(*other.arg));
	}

	std::string toString() const																			// ура узел в строку
	{
		switch (type)
		{
		case NUM: return std::to_string(num);
		case VAR: return name;
		case BIN_OP: return "(" + left->toString() + " " + op + " " + right->toString() + ")";
		case UNARY_OP: return "(" + std::string(1, op) + arg->toString() + ")";
		case FUNC: return name + "(" + arg->toString() + ")";
		}
		return "";
	}
};