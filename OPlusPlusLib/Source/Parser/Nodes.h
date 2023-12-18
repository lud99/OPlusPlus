#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "../macro.h"
#include "Operators.h"
#include "Semantics/SymbolTypeTable.h"

namespace O::AST
{
	enum class NodeKind {
		EmptyStatement,

		Program,

		BasicType,
		ArrayType,
		TupleType,
		FunctionType,

		Identifier,
		VariableDeclaration,

		BinaryExpression,
		UnaryExpression,
		CallExpression,
		TupleExpression,

		FunctionParameters,
		FunctionDefinition,
		ExpressionFunctionDefinition,
		LambdaExpression,

		BlockStatement,

		IfStatement,
		WhileStatement,
		ForStatement,
		LoopStatement,

		Closure,

		Continue,
		Break,

		Return,

		ClassDeclaration,

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

		NodeKind m_Type = NodeKind::EmptyStatement;
		
		EXPORT virtual std::string TypeToString();
		EXPORT virtual void Print(std::string padding = "");
		EXPORT virtual std::string ToString() { return ""; };
	};
};

namespace O::AST::Nodes
{
	struct Scope : public Node
	{
		std::vector<Node*> m_Lines;

		SymbolTypeTable m_LocalTable;

		virtual void Print(std::string padding) override;
	};
	struct Program : public Scope
	{
		Program() { m_Type = NodeKind::Program; };
	};
	struct BlockStatement : public Scope
	{
		BlockStatement() { m_Type = NodeKind::BlockStatement; };
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
		SingleKeywordStatement(NodeKind type) { m_Type = type; };
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

	// Any sort of type
	struct Type : public Node 
	{
		bool m_IsNullable = false;
	};

	// Represents basic types. int, float, Animal, int?[] etc
	struct BasicType : public Type
	{
		BasicType(const std::string& typeName);

		std::string m_TypeName;
	
		std::string ToString() override;
		//void Print(std::string padding) override;
	};
	struct ArrayType : public Type
	{
		ArrayType(Type* underlyingType);

		Type* m_UnderlyingType;

		//std::string ToString() override;
		void Print(std::string padding) override;
	};

	struct TupleType : public Type
	{
		TupleType(std::vector<Type*> elements);

		std::vector<Type*> m_Elements;

		void Print(std::string padding) override;
	};
	struct FunctionType : public Type
	{
		FunctionType(std::vector<Type*> parameters, Type* returnType);

		std::vector<Type*> m_Parameters;
		Type* m_ReturnType;

		void Print(std::string padding) override;
	};

	struct VariableDeclaration : public Node
	{
		VariableDeclaration(Identifier* variableName, Type* variableType, Node* assignedValue);

		Identifier* m_VariableName;
		Type* m_VariableType;
		Node* m_AssignedValue;

		virtual void Print(std::string padding) override;
	};

	struct FunctionParameters : public Node
	{
		FunctionParameters(std::vector<VariableDeclaration*> parameters);

		std::vector<VariableDeclaration*> m_Parameters;

		virtual void Print(std::string padding) override;
	};

	struct FunctionDefinitionStatement : public Node
	{
		FunctionDefinitionStatement(Type* returnType, Identifier* name, FunctionParameters* parameters, Node* body);

		Type* m_ReturnType;
		Identifier* m_Name;
		FunctionParameters* m_Parameters;

		SymbolTypeTable m_ParametersTable;

		Node* m_Body = nullptr;

		bool IsPrototype() { return m_Body == nullptr; }
		bool IsExpressionFunction() { return m_Body->m_Type != NodeKind::BlockStatement; }

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

	struct TupleExpression : public Expression
	{
		TupleExpression(std::vector<Node*> elements);

		std::vector<Node*> m_Elements;

		virtual void Print(std::string padding) override;
	};

	struct CallExpression : public Expression
	{
		CallExpression(Node* callee, TupleExpression* arguments);

		Node* m_Callee; // The 'name' of the function called
		TupleExpression* m_Arguments; // TODO: Not semantically correct

		virtual void Print(std::string padding) override;
	};

	struct LambdaExpression : public Node
	{
		LambdaExpression(Type* returnType, TupleExpression* parameters, Node* body);

		Type* m_ReturnType;
		TupleExpression* m_Parameters;

		Node* m_Body = nullptr;

		virtual void Print(std::string padding) override;
	};



	struct ClassDeclarationStatement;
	struct ClassDeclarationStatement : public Node
	{
		ClassDeclarationStatement(Identifier* name);

		Identifier* m_Name;

		std::vector<VariableDeclaration*> m_MemberDeclarations;
		std::vector<FunctionDefinitionStatement*> m_MethodDeclarations;
		std::vector<ClassDeclarationStatement*> m_NestedClassDeclarations;

		ClassSymbol* m_ClassSymbol = nullptr;
		//SymbolTable m_MemberSymbolTable;

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