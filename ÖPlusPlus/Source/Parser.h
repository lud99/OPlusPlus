#pragma once

#include <vector>
#include <assert.h>

#include "Lexer.h"

struct ASTNode;

enum class ASTTypes
{
	Empty,
	VariableDeclaration,
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
	StringLiteral,
	Bool,
	ArrayType,
	FunctionType,
	ObjectType,
	Variable,
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
	AnonymousFunction,
	Break,
	Continue,
	Import,
	Export,
	Scope,
	Modifier
};

struct ASTNode
{
	ASTNode* parent = nullptr;
	ASTNode* left = nullptr;
	ASTNode* right = nullptr;
	std::vector<ASTNode*> arguments;

	ASTTypes type = ASTTypes::Empty;

	// optional
	float numberValue = 0.0f;
	std::string stringValue = "";

	std::string ToString(bool includeData = true);

	ASTNode() {};
	ASTNode(ASTTypes _type, ASTNode* _left = nullptr, ASTNode* _right = nullptr) : type(_type), left(_left), right(_right) {};

	inline bool IsMathOperator()
	{ 
		return type == ASTTypes::Add || type == ASTTypes::Subtract || type == ASTTypes::Multiply || type == ASTTypes::Divide ||
			   type == ASTTypes::Modulus || type == ASTTypes::Xor || type == ASTTypes::ToThePower;
	}

	inline bool IsPrimitiveType()
	{
		abort();
		return false;// return type == ASTTypes::Number || type == ASTTypes::String || type == ASTTypes::ArrayType || type == ASTTypes::FunctionType;
	}
};

class Parser
{
public:
	bool MakeError(const std::string& message);
	void MakeErrorVoid(const std::string& message);

	void PrintASTTree(ASTNode* node, int depth);

	bool IsValidAssignmentExpression(std::vector<Token> tokens, int equalsSignPosition);
	//bool IsValidPropertyAssignmentExpression(std::vector<Token> tokens);
	bool IsValidVariableDeclarationExpression(std::vector<Token> tokens);
	bool IsValidCompoundAssignment(std::vector<Token> tokens, int operatorPosition);
	bool IsValidPostIncDecExpression(std::vector<Token> tokens);
	bool IsValidPreIncDecExpression(std::vector<Token> tokens);
	//bool IsValidModifier(std::vector<Token> tokens);


	void CreateAST(std::vector<Token>& tokens, ASTNode* node, ASTNode* parent = nullptr);

	bool ParseAssignment(Tokens& tokens, ASTNode* node);
	bool ParseLogicalAndOr(Tokens& tokens, ASTNode* node);
	bool ParseComparisonOperators(Tokens& tokens, ASTNode* node);
	bool ParsePlusMinusEquals(Tokens& tokens, ASTNode* node);
	bool ParseMathExpression(Tokens& tokens, ASTNode* node);
	bool ParseVariableDeclaration(Tokens& tokens, ASTNode* node);
	bool ParseParentheses(Tokens& tokens, ASTNode* node);
	bool ParseIncrementDecrement(Tokens& tokens, ASTNode* node);

public:
	std::string m_Error = "";

	ASTNode root;
};