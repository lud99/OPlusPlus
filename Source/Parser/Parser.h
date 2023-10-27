#pragma once

#include "../macro.h"

#include <vector>
#include <set>
#include <unordered_map>
#include <assert.h>
#include <optional>
#include <deque>

#include "Nodes.h"
#include "Lexer.h"
#include "Operators.h"
//#include "TypeTable.h"

namespace Ö::AST
{
	typedef std::vector<Tokens> LinesOfTokens;

	struct PrefixParselet;
	struct InfixParselet;

	class Parser
	{
	public:
		EXPORT Parser(Tokens& tokens);

		EXPORT Node CreateRootNode();

		EXPORT Node* ParseExpression(int precedence = 0);

		//EXPORT Node* CreateAST(Tokens& tokens, Node* parent);
		
		EXPORT float TemporaryEvaluator(Node* node);

		Node* MakeError(const std::string& message);
		void MakeErrorVoid(const std::string& message);

		EXPORT bool HasError() { return m_Error != ""; }
		EXPORT const std::string& GetError() { return m_Error; };

		// Lexer functions
		Token ConsumeToken();
		std::optional<Token> ConsumeToken(Token::Types expectedType);
		Token PeekToken(int distance);
		bool MatchToken(Token::Types expectedType);
		int GetPrecedenceOfCurrentToken();

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
		Node* ParseParentheses(Tokens& tokens, Node* parent);
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
		bool ParseMemberAccessor(Tokens& tokens, Node* node);
		bool ParseScopeResolution(Tokens& tokens, Node* node);
		bool ParseFunctionCall(Tokens& tokens, Node* node);
		bool ParseIncrementDecrement(Tokens& tokens, Node* node);

		LinesOfTokens MakeScopeIntoLines(Tokens tokens, int start, int end, int startingDepth);
		bool IsInsideBrackets(Tokens tokens, int start);

		// Find the matching bracket (, {, [ with the same depth
		std::optional<int> FindMatchingEndBracket(Tokens& tokens, Token& startToken);
		std::optional<int> FindMatchingStartBracket(Tokens& tokens, Token& endToken);

	private:
		std::string m_Error = "";

		Tokens m_Tokens;
		std::deque<Token> m_TokenStream;

		Node root;

		std::unordered_map<Token::Types, PrefixParselet*> m_PrefixParselets;
		std::unordered_map<Token::Types, InfixParselet*> m_InfixParselets;

	public:
		Operators::DefinedOperators m_DefinedOperators;
	};

	// Base classes
	struct PrefixParselet
	{
		virtual Node* Parse(Parser& parser, Token token, Node* parent) { abort();  return nullptr; }
	};
	struct InfixParselet
	{
		virtual Node* Parse(Parser& parser, Node* left, Token token, Node* parent) { abort();  return nullptr; }
	};

	// Identifiers and literals
	struct IdentifierParselet : public PrefixParselet
	{
		Node* Parse(Parser& parser, Token token, Node* parent) override;
	};
	struct LiteralParselet : public PrefixParselet
	{
		Node* Parse(Parser& parser, Token token, Node* parent) override;
	};	
	struct ParenthesesGroupParselet : public PrefixParselet
	{
		Node* Parse(Parser& parser, Token token, Node* parent) override;
	};


	// Operators

	struct PrefixOperatorParselet : public PrefixParselet 
	{
		Node* Parse(Parser& parser, Token token, Node* parent) override;
	};
	struct PostfixOperatorParselet : public InfixParselet
	{
		Node* Parse(Parser& parser, Node* left, Token token, Node* parent) override;
	};
	struct BinaryOperatorParselet : public InfixParselet
	{
		Node* Parse(Parser& parser, Node* left, Token token, Node* parent) override;
	};
	struct CallParselet : public InfixParselet
	{
		Node* Parse(Parser& parser, Node* left, Token token, Node* parent) override;
	};
}