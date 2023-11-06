#pragma once

#include "Parser.h"

namespace O::AST
{

	// Base classes
	struct PrefixParselet
	{
		virtual Node* Parse(Parser& parser, Token token) { abort();  return nullptr; }
	};
	struct InfixParselet
	{
		virtual Node* Parse(Parser& parser, Node* left, Token token) { abort();  return nullptr; }
	};
	struct StatementParselet
	{
		virtual Node* Parse(Parser& parser, Token token) { abort();  return nullptr; }
	};

	// Identifiers and literals
	struct IdentifierParselet : public PrefixParselet
	{
		Node* Parse(Parser& parser, Token token) override;
	};
	struct LiteralParselet : public PrefixParselet
	{
		Node* Parse(Parser& parser, Token token) override;
	};
	struct ParenthesesGroupParselet : public PrefixParselet
	{
		Node* Parse(Parser& parser, Token token) override;
	};


	// Operators

	struct PrefixOperatorParselet : public PrefixParselet
	{
		Node* Parse(Parser& parser, Token token) override;
	};
	struct PostfixOperatorParselet : public InfixParselet
	{
		Node* Parse(Parser& parser, Node* left, Token token) override;
	};
	struct ClosureParselet : public PrefixParselet
	{
		Node* Parse(Parser& parser, Token token) override;
	};

	struct BinaryOperatorParselet : public InfixParselet
	{
		Node* Parse(Parser& parser, Node* left, Token token) override;
	};
	struct CallParselet : public InfixParselet
	{
		Node* Parse(Parser& parser, Node* left, Token token) override;
	};
	struct LambdaParselet : public InfixParselet
	{
		Node* Parse(Parser& parser, Node* left, Token token) override;
	};

	// Statements
	struct BlockStatementParselet : public StatementParselet
	{
		Node* Parse(Parser& parser, Token token) override;
	};
	struct ConditionalStatementParselet : public StatementParselet
	{
		Node* Parse(Parser& parser, Token token) override;
	};
	struct ForStatementParselet : public StatementParselet
	{
		Node* Parse(Parser& parser, Token token) override;
	};
	struct LoopParselet : public PrefixParselet
	{
		Node* Parse(Parser& parser, Token token) override;
	};

	// continue, break, return
	struct SingleKeywordParselet : public StatementParselet
	{
		Node* Parse(Parser& parser, Token token) override;
	};
	struct BreakParselet : public StatementParselet
	{
		Node* Parse(Parser& parser, Token token) override;
	};
	struct ReturnParselet : public StatementParselet
	{
		Node* Parse(Parser& parser, Token token) override;
	};

	struct TypenameStatementParselet : public StatementParselet
	{
		Node* Parse(Parser& parser, Token token) override;
	};

	struct ClassDefinitionParselet : public StatementParselet
	{
		Node* Parse(Parser& parser, Token token) override;
	};
}