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
		Nodes::Type* ParseType(int precedence = 0);

		Nodes::Type* ParseType(Token token, int precedence = 0);
		Nodes::Type* ParseTypeAnnotation();
		Nodes::Identifier* ParseIdentifier(Token token);
		std::vector<Node*> ParseTupleLikeExpression(Token token);

		Nodes::VariableDeclaration* ParseVariableDeclaration(Token token, bool consumeEndToken = true, std::vector<Token::Types> endTokens = { Token::Semicolon });
		Nodes::FunctionParameters* ParseFunctionParameters(Token token);

		EXPORT float TemporaryEvaluator(Node* node);

		Node* MakeError(const std::string& message, Token errorToken, CompileTimeError::Severity severity = CompileTimeError::Error);
		Node* MakeError(const std::string& message, CompileTimeError::Severity severity = CompileTimeError::Error);

		template <typename T>
		T* Insert(T* node, TokenRange range);
		template <typename T>
		T* Insert(T* node, Node* startNode); // Automatic end
		template <typename T>
		T* Insert(T* node, Token startToken); // Automatic end

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

		bool TokenIsTypename(Token token) { return m_TypeTable.HasType(token.m_Value); }
		bool TokenIsIdentifier(Token token) { return !TokenIsTypename(token) && token.m_Type == Token::Identifier; }

		auto& GetTokens() { return m_Tokens; };

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
		TypeTable m_TypeTable = TypeTable(TypeTableType::Global, nullptr);

		std::unordered_map<Node*, TokenRange> m_NodesToTokesMappings;
	};

	template<typename T>
	inline T* Parser::Insert(T* node, TokenRange range)
	{
		if (!node) return node;

		assert(m_NodesToTokesMappings.count(node) == 0);

		m_NodesToTokesMappings[node] = range;
		return node;
	}
	template<typename T>
	inline T* Parser::Insert(T* node, Node* startNode)
	{
		assert(m_NodesToTokesMappings.count(node) == 0);
		assert(m_NodesToTokesMappings.count(startNode) == 1);

		return Insert(node, { m_NodesToTokesMappings[startNode].m_Start, m_LastConsumedToken.m_StartPosition });
	}
	template<typename T>
	inline T* Parser::Insert(T* node, Token startToken)
	{
		return Insert(node, { startToken.m_StartPosition, m_LastConsumedToken.m_StartPosition });
	}
}