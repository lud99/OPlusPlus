#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "../macro.h"
#include "Operators.h"

namespace Ö::AST
{
	struct Node;

	enum class NodeType {
		Empty,

		Root,
		Program,

		Identifier,
		VariableDeclaration,

		AssignmentExpression,
		BinaryExpression,
		UnaryExpression,
		CallExpression,

		BlockStatement,

		IntLiteral,
		FloatLiteral,
		DoubleLiteral,
		StringLiteral
	};
	/*{
		Empty,
		VariableDeclaration,
		GlobalVariableDeclaration,
		VariableType,
		Assign,
		PropertyAssign,
		CompareEquals,
		CompareNotEquals,
		CompareLessThan,
		CompareGreaterThan,
		CompareLessThanEqual,
		CompareGreaterThanEqual,
		And,
		Or,
		Not,
		Null,
		IntLiteral,
		DoubleLiteral,
		StringLiteral,
		Bool,
		ArrayType,
		FunctionType,
		ObjectType,
		Class,
		Variable,
		MemberAcessor,
		ScopeResolution,
		PropertyAccess,
		ListInitializer,
		MathExpression,
		Add,
		Subtract,
		Multiply,
		Divide,
		Xor,
		ToThePower,
		Modulus,
		PlusEquals,
		MinusEquals,
		PostIncrement,
		PreIncrement,
		PostDecrement,
		PreDecrement,
		ProgramBody,
		ModuleBody,
		Line,
		FunctionCall,
		Return,
		IfStatement,
		Else,
		WhileStatement,
		ForStatement,
		FunctionDefinition,
		FunctionPrototype,
		Break,
		Continue,
		Import,
		Export,
		Scope,
		Modifier
	};*/

	struct Node;
	extern "C" struct Node
	{
		EXPORT Node();
		EXPORT Node(Node* parent);

		Node* m_Parent = nullptr;
		NodeType m_Type = NodeType::Empty;

		EXPORT virtual std::string TypeToString()
		{
			return std::string(magic_enum::enum_name(m_Type));
		};

		EXPORT virtual void Print(std::string padding = "")
		{
			std::cout << padding << TypeToString() << (ToString() != "" ? ": " : "") << ToString() << "\n";
		}

		EXPORT virtual std::string ToString() { return ""; };
	};

	struct Scope : public Node
	{
		std::vector<Node*> m_Lines;

		virtual void Print(std::string padding) override
		{
			std::cout << padding << TypeToString() << ": \n";
			for (auto& line : m_Lines)
				line->Print(padding + "    ");
		}
	};
	struct Program : public Scope
	{
		Program(Node* parent);
	};
	struct BlockStatement : public Scope
	{
		BlockStatement(Node* parent);
	};

	struct Identifier : public Node
	{
		Identifier(Node* parent, const std::string& name);

		std::string m_Name;

		std::string ToString() override { return m_Name; }
	};

	struct Type : public Node
	{
		Type(Node* parent, const std::string& typeName);

		std::string m_TypeName;

		std::string ToString() override { return m_TypeName; }
	};

	struct VariableDeclaration : public Node
	{
		VariableDeclaration(Node* parent);

		Type* m_TypeNode;
		Identifier* m_IdentifierNode;
		Node* m_ValueNode;
	};

	struct Expression : public Node { };

	struct BinaryExpression : public Expression
	{
		BinaryExpression(Node* parent, Node* left, Operators::Operator op, Node* right);

		Node* m_Lhs;
		Operators::Operator m_Operator;
		Node* m_Rhs;

		virtual void Print(std::string padding) override
		{
			std::cout << padding << TypeToString() << " (" << ToString() << "):\n";
			m_Lhs->Print(padding + "    ");
			m_Rhs->Print(padding + "    ");
		}

		std::string ToString() override
		{
			return m_Operator.m_Symbol;
		}
	};

	struct UnaryExpression : public Expression
	{
		UnaryExpression(Node* parent, Node* operand, Operators::Operator op);

		Node* m_Operand;
		Operators::Operator m_Operator;

		virtual void Print(std::string padding) override
		{
			if (m_Operator.m_Associaticity == Operators::Associativity::Left)
			{
				m_Operand->Print(padding + "    ");
				std::cout << padding << TypeToString() << " (" << ToString() << "):\n";
			}
			else if (m_Operator.m_Associaticity == Operators::Associativity::Right)
			{
				std::cout << padding << TypeToString() << " (" << ToString() << "):\n";
				m_Operand->Print(padding + "    ");
			}
		}

		std::string ToString() override
		{
			return m_Operator.m_Symbol;
		}
	};

	struct CallExpression : public Expression
	{
		CallExpression(Node* parent, Node* callee, std::vector<Node*> arguments);

		Node* m_Callee;
		std::vector<Node*> m_Arguments;

		virtual void Print(std::string padding) override
		{
			std::cout << padding << TypeToString() << " (" << m_Callee->ToString() << "):\n";

			for (auto& argument : m_Arguments)
			{
				argument->Print(padding + "    ");
			}	
		}
	};

	class Literal : public Node {};
	struct IntLiteral : public Literal
	{
		IntLiteral(Node* parent, int value);

		int m_Value;

		std::string ToString() override { return std::to_string(m_Value); }
	};
	struct FloatLiteral : public Literal
	{
		FloatLiteral(Node* parent, float value);

		float m_Value;

		std::string ToString() override { return std::to_string(m_Value); }
	};
	struct DoubleLiteral : public Literal
	{
		DoubleLiteral(Node* parent, double value);

		double m_Value;

		std::string ToString() override { return std::to_string(m_Value); }
	};
	struct StringLiteral : public Literal
	{
		StringLiteral(Node* parent, std::string value);

		std::string m_Value;

		std::string ToString() override { return m_Value; }
	};

}