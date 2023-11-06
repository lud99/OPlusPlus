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

namespace O::AST
{
	using namespace O::Lexer;
	typedef std::vector<Tokens> LinesOfTokens;

	struct PrefixParselet;
	struct InfixParselet;
	struct StatementParselet;

	struct ParserError
	{
		enum Severity {
			Info,
			Warning,
			Error
		};

		Severity severity;
		std::string message;

		Token token;
	};

	class Parser
	{
	public:
		EXPORT Parser(Tokens& tokens);

		EXPORT Node* ParseProgram();
		
		Node* Parse();
		Node* ParseExpression(int precedence = 0);

		std::tuple<Type*, Identifier*> ParseTypeAndName(Token token);
		VariableDeclaration* ParseVariableDeclaration(Token token, Type* type, Identifier* name, bool consumeEndToken = true, Token::Types endToken = Token::Semicolon);
		std::vector<VariableDeclaration*> ParseFunctionParameters(Token token);

		EXPORT float TemporaryEvaluator(Node* node);

		Node* MakeErrorButPretty(const std::string& message, ParserError::Severity severity = ParserError::Error);
		Node* MakeErrorButPretty(const std::string& message, Token errorToken, ParserError::Severity severity = ParserError::Error);

		EXPORT bool HasError() { return !m_Errors.empty(); }
		EXPORT auto& GetErrors() { return m_Errors; };

		EXPORT void PrintErrors();

		// Lexer functions
		Token ConsumeToken();
		std::optional<Token> ConsumeToken(Token::Types expectedType);
		
		Token PeekToken(int distance = 0);
		
		bool MatchToken(Token::Types expectedType);
		bool MatchTokenNoConsume(Token::Types expectedType) { return MatchTokenNoConsume(0, expectedType); };

		bool MatchTokenNoConsume(int peekDistance, Token::Types expectedType);

		bool EnsureToken(int peekDistance, Token::Types expectedType);

		int GetPrecedenceOfCurrentToken();

		bool TokenIsTypename(Token token) { return m_TypeTable.HasType(token.m_Value); }
		bool TokenIsIdentifier(Token token) { return !TokenIsTypename(token) && token.m_Type == Token::Identifier; }

	private:
		std::vector<ParserError> m_Errors;

		Tokens m_Tokens;
		std::deque<Token> m_TokenStream;
		Token m_LastConsumedToken;

		Node root;

		std::unordered_map<Token::Types, PrefixParselet*> m_PrefixParselets;
		std::unordered_map<Token::Types, InfixParselet*> m_InfixParselets;

		std::unordered_map<Token::Types, StatementParselet*> m_StatementParselets;

	public:
		Operators::DefinedOperators m_DefinedOperators;
		TypeTable m_TypeTable;
	};
}