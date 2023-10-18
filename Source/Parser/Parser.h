#pragma once

#include "../macro.h"

#include <vector>
#include <set>
#include <unordered_map>
#include <assert.h>
#include <optional>

#include "Nodes.h"
#include "Lexer.h"
#include "Operators.h"
//#include "TypeTable.h"

namespace Ö::AST
{
	typedef std::vector<Tokens> LinesOfTokens;

	class Parser
	{
	public:
		EXPORT Parser();

		EXPORT Node CreateRootNode();

		EXPORT Node* CreateAST(Tokens& tokens, Node* parent);
		
		EXPORT void PrintASTTree(Node* node, int depth);

		EXPORT bool HasError() { return m_Error != ""; }
		EXPORT const std::string& GetError() { return m_Error; };

		//TypeTable GetGeneratedTypeTable() { return m_TypeTable; };

	private:
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

		Node* ParseScope(Tokens& tokens, Node* parent);
		Node* ParseBinaryExpression(Tokens& tokens, Node* parent);

		bool ParseClassDeclaration(Tokens& tokens, Node* node);
		bool ParseElseStatement(Tokens& tokens, Node* node);
		bool ParseStatement(Tokens& tokens, Node* node);
		bool ParseFunctionDeclaration(Tokens& tokens, Node* node);
		bool ParseFunctionPrototype(Tokens& tokens, Node* node);
		bool ParseAssignment(Tokens& tokens, Node* node);
		bool ParseLogicalAndOr(Tokens& tokens, Node* node);
		bool ParseComparisonOperators(Tokens& tokens, Node* node);
		bool ParseCompoundAssignment(Tokens& tokens, Node* node);
		bool ParseMathExpression(Tokens& tokens, Node* node);
		bool ParseVariableDeclaration(Tokens& tokens, Node* node);
		bool ParsePropertyAccessExpression(Tokens& tokens, Node* node);
		bool ParseParentheses(Tokens& tokens, Node* node);
		bool ParseMemberAccessor(Tokens& tokens, Node* node);
		bool ParseScopeResolution(Tokens& tokens, Node* node);
		bool ParseFunctionCall(Tokens& tokens, Node* node);
		bool ParseIncrementDecrement(Tokens& tokens, Node* node);

		LinesOfTokens MakeScopeIntoLines(Tokens tokens, int start, int end, int startingDepth);
		bool IsInsideBrackets(Tokens tokens, int start);

		Node* MakeError(const std::string& message);
		void MakeErrorVoid(const std::string& message);

	private:
		std::string m_Error = "";

		Node root;

		Operators::DefinedOperators m_DefinedOperators;
	};

}