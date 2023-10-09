#pragma once

#include "macro.h"

#include <vector>
#include <set>
#include <unordered_map>
#include <assert.h>

#include "Lexer.h"
#include "TypeTable.h"
#include "Interpreter/ValueTypes.h"

namespace Ö
{
	struct ASTNode;

	enum class ASTTypes
	{
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
	};

	extern "C" struct ASTNode
	{
		ASTNode* parent = nullptr;
		ASTNode* left = nullptr;
		ASTNode* right = nullptr;
		std::vector<ASTNode*> arguments;

		ASTTypes type = ASTTypes::Empty;

		// optional
		float numberValue = 0.0f;
		std::string stringValue = "";

		EXPORT std::string ToString(bool includeData = true);

		ASTNode() {};
		ASTNode(ASTTypes _type, ASTNode* _left = nullptr, ASTNode* _right = nullptr) : type(_type), left(_left), right(_right) {};

		inline bool IsMathOperator()
		{
			return type == ASTTypes::Add || type == ASTTypes::Subtract || type == ASTTypes::Multiply || type == ASTTypes::Divide ||
				type == ASTTypes::Modulus || type == ASTTypes::Xor || type == ASTTypes::ToThePower;
		}

		inline bool IsCompoundAssignment()
		{
			return type == ASTTypes::PlusEquals || type == ASTTypes::MinusEquals;
		}
		inline bool IsAssignment()
		{
			return IsCompoundAssignment() || type == ASTTypes::Assign;
		}

		inline bool IsPrimitiveType()
		{
			abort();
			return false;// return typeEntry == ASTTypes::Number || typeEntry == ASTTypes::String || typeEntry == ASTTypes::ArrayType || typeEntry == ASTTypes::FunctionType;
		}

		/*inline ValueTypes VariableTypeToValueType()
		{
			assert(typeEntry == ASTTypes::VariableType);

			if (stringValue == "int") return ValueTypes::Integer;
			if (stringValue == "float") return ValueTypes::Float;
			if (stringValue == "double") return ValueTypes::Float;
			if (stringValue == "string") return ValueTypes::String;

			return ValueTypes::Void;
		}*/
	};

	class Parser
	{
	public:
		EXPORT ASTNode CreateRootNode();

		EXPORT void CreateAST(Tokens& tokens, ASTNode* node, ASTNode* parent = nullptr);
		
		EXPORT void PrintASTTree(ASTNode* node, int depth);

		EXPORT bool HasError() { return m_Error != ""; }
		EXPORT const std::string& GetError() { return m_Error; };

		TypeTable GetGeneratedTypeTable() { return m_TypeTable; };

	private:
		bool MakeError(const std::string& message);
		void MakeErrorVoid(const std::string& message);

		bool IsValidClassDeclaration(Tokens tokens);
		bool IsValidElseStatement(Tokens tokens, int position);
		bool IsValidStatement(Tokens tokens);
		bool IsValidAssignmentExpression(Tokens tokens, int equalsSignPosition);
		bool IsValidCompoundAssignmentExpression(Tokens tokens, int position);
		bool IsValidComparisonExpression(Tokens tokens, int position);
		bool IsValidLogicalAndOrExpression(Tokens tokens, int position);
		bool IsValidPropertyAccessExpression(Tokens tokens);
		bool IsValidPostIncDecExpression(Tokens tokens, int position);
		bool IsValidPreIncDecExpression(Tokens tokens, int position);
		bool IsValidScopeResolutionExpresion(Tokens tokens, int position);
		bool IsValidFunctionCallExpression(Tokens tokens);
		bool IsValidVariableDeclarationExpression(Tokens tokens);

		bool ParseClassDeclaration(Tokens& tokens, ASTNode* node);
		bool ParseElseStatement(Tokens& tokens, ASTNode* node);
		bool ParseStatement(Tokens& tokens, ASTNode* node);
		bool ParseFunctionDeclaration(Tokens& tokens, ASTNode* node);
		bool ParseFunctionPrototype(Tokens& tokens, ASTNode* node);
		bool ParseAssignment(Tokens& tokens, ASTNode* node);
		bool ParseLogicalAndOr(Tokens& tokens, ASTNode* node);
		bool ParseComparisonOperators(Tokens& tokens, ASTNode* node);
		bool ParseCompoundAssignment(Tokens& tokens, ASTNode* node);
		bool ParseMathExpression(Tokens& tokens, ASTNode* node);
		bool ParseVariableDeclaration(Tokens& tokens, ASTNode* node);
		bool ParsePropertyAccessExpression(Tokens& tokens, ASTNode* node);
		bool ParseParentheses(Tokens& tokens, ASTNode* node);
		bool ParseMemberAccessor(Tokens& tokens, ASTNode* node);
		bool ParseScopeResolution(Tokens& tokens, ASTNode* node);
		bool ParseFunctionCall(Tokens& tokens, ASTNode* node);
		bool ParseIncrementDecrement(Tokens& tokens, ASTNode* node);

	private:
		std::string m_Error = "";

		ASTNode root;

		TypeTable m_TypeTable;
	};

}