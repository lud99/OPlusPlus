#pragma once

#include "../macro.h"

#include <vector>
#include <set>
#include <unordered_map>
#include <assert.h>
#include <optional>
#include <deque>

#include "Semantics/TypeTable.h"
#include "Nodes.h"
#include "Lexer.h"
#include "Operators.h"

namespace Ö::AST
{
	typedef std::vector<Tokens> LinesOfTokens;

	struct PrefixParselet;
	struct InfixParselet;
	struct StatementParselet;

	class Parser
	{
	public:
		EXPORT Parser(Tokens& tokens);

		EXPORT Node* ParseProgram();
		
		Node* Parse();
		Node* ParseExpression(int precedence = 0);

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
		bool MatchTokenNoConsume(Token::Types expectedType) { return MatchTokenNoConsume(0, expectedType); };

		bool MatchTokenNoConsume(int peekDistance, Token::Types expectedType);

		bool EnsureToken(int peekDistance, Token::Types expectedType);

		int GetPrecedenceOfCurrentToken();

		bool TokenIsTypename(Token token) { return m_TypeTable.HasType(token.m_Value); }
		bool TokenIsIdentifier(Token token) { return !TokenIsTypename(token) && token.m_Type == Token::Identifier; }

	private:
		std::string m_Error = "";

		Tokens m_Tokens;
		std::deque<Token> m_TokenStream;

		Node root;

		std::unordered_map<Token::Types, PrefixParselet*> m_PrefixParselets;
		std::unordered_map<Token::Types, InfixParselet*> m_InfixParselets;

		std::unordered_map<Token::Types, StatementParselet*> m_StatementParselets;

	public:
		Operators::DefinedOperators m_DefinedOperators;
		TypeTable m_TypeTable;
	};
}