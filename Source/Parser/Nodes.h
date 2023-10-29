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

		Typename,
		Identifier,
		VariableDeclaration,

		AssignmentExpression,
		BinaryExpression,
		UnaryExpression,
		CallExpression,

		BlockStatement,

		FunctionDefinition,

		IfStatement,
		WhileStatement,
		ForStatement,
		LoopStatement,

		Closure,

		Continue,
		Break,

		Return,

		IntLiteral,
		FloatLiteral,
		DoubleLiteral,
		BoolLiteral,
		StringLiteral
	};

	struct Node;
	extern "C" struct Node
	{
		EXPORT Node() {};

		NodeType m_Type = NodeType::Empty;

		EXPORT virtual std::string TypeToString();
		EXPORT virtual void Print(std::string padding = "");
		EXPORT virtual std::string ToString() { return ""; };
	};

	struct Scope : public Node
	{
		std::vector<Node*> m_Lines;

		virtual void Print(std::string padding) override;
	};
	struct Program : public Scope
	{
		Program() { m_Type = NodeType::Program; };
	};
	struct BlockStatement : public Scope
	{
		BlockStatement() { m_Type = NodeType::BlockStatement; };
	};



	struct ConditionalStatement : public Node
	{
		Node* m_Condition;
		BlockStatement* m_Body;

		virtual void Print(std::string padding) override;
	};
	struct WhileStatement : public ConditionalStatement
	{
		WhileStatement(Node* condition, BlockStatement* body);
	};
	struct IfStatement : public ConditionalStatement
	{
		IfStatement(Node* condition, BlockStatement* body, BlockStatement* elseArm);
		
		BlockStatement* m_ElseArm;

		virtual void Print(std::string padding) override;
	};
	struct ForStatement : public Node
	{
		ForStatement(Node* initialization, Node* condition, Node* advancement, BlockStatement* body);
		
		Node* m_Initialization;
		Node* m_Condition;
		Node* m_Advancement;

		BlockStatement* m_Body;

		virtual void Print(std::string padding) override;
	};
	struct LoopStatement : public Node
	{
		LoopStatement(BlockStatement* body);

		BlockStatement* m_Body;

		virtual void Print(std::string padding) override;
	};

	struct ClosureExpression : public Node
	{
		ClosureExpression(BlockStatement* body);

		BlockStatement* m_Body;
		
		virtual void Print(std::string padding) override;
	};

	struct SingleKeywordStatement : public Node
	{
		SingleKeywordStatement(NodeType type) { m_Type = type; };
	};
	struct ReturnStatement : public Node
	{
		ReturnStatement(Node* returnValue);

		Node* m_ReturnValue;

		virtual void Print(std::string padding) override;
	};
	struct BreakStatement : public Node
	{
		BreakStatement(Node* breakValue);

		Node* m_BreakValue;

		virtual void Print(std::string padding) override;
	};

	struct Identifier : public Node
	{
		Identifier(const std::string& name);

		std::string m_Name;

		std::string ToString() override { return m_Name; }
	};


	struct Type : public Node
	{
		Type(const std::string& typeName);

		std::string m_TypeName;

		std::string ToString() override { return m_TypeName; }
	};

	struct VariableDeclaration : public Node
	{
		VariableDeclaration(Type* variableType, Identifier* variableName, Node* assignedValue);

		Type* m_VariableType;
		Identifier* m_VariableName;
		Node* m_AssignedValue;

		virtual void Print(std::string padding) override;
	};

	struct FunctionDefinitionStatement : public Node
	{
		FunctionDefinitionStatement(Type* returnType, Identifier* name, std::vector<Node*> parameters, BlockStatement* body);

		Type* m_ReturnType;
		Identifier* m_Name;
		std::vector<Node*> m_Parameters;

		BlockStatement* m_Body = nullptr;

		bool IsPrototype() { return m_Body == nullptr; }

		virtual void Print(std::string padding) override;
	};

	struct Expression : public Node { };

	struct BinaryExpression : public Expression
	{
		BinaryExpression(Node* left, Operators::Operator op, Node* right);

		Node* m_Lhs;
		Operators::Operator m_Operator;
		Node* m_Rhs;

		virtual void Print(std::string padding) override;
		std::string ToString() override;
	};

	struct UnaryExpression : public Expression
	{
		UnaryExpression(Node* operand, Operators::Operator op);

		Node* m_Operand;
		Operators::Operator m_Operator;

		virtual void Print(std::string padding) override;
		std::string ToString() override;
	};

	struct CallExpression : public Expression
	{
		CallExpression(Node* callee, std::vector<Node*> arguments);

		Node* m_Callee;
		std::vector<Node*> m_Arguments;

		virtual void Print(std::string padding) override;
	};

	class Literal : public Node {};
	struct IntLiteral : public Literal
	{
		IntLiteral(int value);

		int m_Value;

		std::string ToString() override { return std::to_string(m_Value); }
	};
	struct FloatLiteral : public Literal
	{
		FloatLiteral(float value);

		float m_Value;

		std::string ToString() override { return std::to_string(m_Value); }
	};
	struct DoubleLiteral : public Literal
	{
		DoubleLiteral(double value);

		double m_Value;

		std::string ToString() override { return std::to_string(m_Value); }
	};
	struct BoolLiteral : public Literal
	{
		BoolLiteral(bool value);

		bool m_Value;

		std::string ToString() override { return m_Value ? "true" : "false"; }
	};
	struct StringLiteral : public Literal
	{
		StringLiteral(std::string value);

		std::string m_Value;

		std::string ToString() override { return "\"" + m_Value + "\""; }
	};

}