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
#include "CompileTimeErrorList.h"

namespace O::AST
{
	using namespace O::Lexer;
	typedef std::vector<Tokens> LinesOfTokens;

	struct PrefixParselet;
	struct InfixParselet;

	struct PrefixTypeParselet;
	struct InfixTypeParselet;
	
	struct StatementParselet;

	class Parser : public CompileTimeErrorList
	{
	public:
		EXPORT Parser(Tokens& tokens);

		EXPORT Node* ParseProgram();
		
		Node* Parse();
		Node* ParseExpression(int precedence = 0);
		Type* ParseType(int precedence = 0);

		Type* ParseType(Token token, int precedence = 0);
		Identifier* ParseIdentifier(Token token);
		TupleExpression* ParseTupleExpression(Token token);

		VariableDeclaration* ParseVariableDeclaration(Token token, Type* type, Identifier* name, bool consumeEndToken = true, Token::Types endToken = Token::Semicolon);
		FunctionParameters* ParseFunctionParameters(Token token);

		EXPORT float TemporaryEvaluator(Node* node);

		Node* MakeError(const std::string& message, Token errorToken, CompileTimeError::Severity severity = CompileTimeError::Error);
		Node* MakeError(const std::string& message, CompileTimeError::Severity severity = CompileTimeError::Error);


		// Lexer functions
		Token ConsumeToken();
		std::optional<Token> ConsumeToken(Token::Types expectedType);
		
		Token PeekToken(int distance = 0);
		
		bool MatchToken(Token::Types expectedType);
		bool MatchTokenNoConsume(Token::Types expectedType) { return MatchTokenNoConsume(0, expectedType); };

		bool MatchTokenNoConsume(int peekDistance, Token::Types expectedType);

		bool EnsureToken(int peekDistance, Token::Types expectedType);

		int GetPrecedenceOfCurrentToken();
		int GetPrecedenceOfCurrentTokenType();

		bool TokenIsTypename(Token token) { return m_TypeTable.Has(token.m_Value); }
		bool TokenIsIdentifier(Token token) { return !TokenIsTypename(token) && token.m_Type == Token::Identifier; }

	private:
		Tokens m_Tokens;
		std::deque<Token> m_TokenStream;
		Token m_LastConsumedToken;

		Node root;

		std::unordered_map<Token::Types, PrefixParselet*> m_PrefixParselets;
		std::unordered_map<Token::Types, InfixParselet*> m_InfixParselets;

		std::unordered_map<Token::Types, PrefixTypeParselet*> m_PrefixTypeParselets;
		std::unordered_map<Token::Types, InfixTypeParselet*> m_InfixTypeParselets;

		std::unordered_map<Token::Types, StatementParselet*> m_StatementParselets;

	public:
		Operators::DefinedOperators m_DefinedOperators;
		Operators::DefinedOperators m_DefinedTypeModifierOperators;
		TypeTable m_TypeTable;
	};
}