#include "Parser.h"

#include <iostream>

#include "Parselets.h"
#include "../Utils.hpp"

#define RETURN_IF_ERROR() if (HasError()) return nullptr;

namespace O::AST
{
	using namespace Nodes;

	Parser::Parser(Tokens& tokens)
	{
		m_Tokens = tokens;
		m_TokenStream = std::deque<Token>(tokens.begin(), tokens.end());

		using namespace Operators;

		// Using the precedences from C++
		// https://en.cppreference.com/w/cpp/language/operator_precedence

		// p = 2
		m_DefinedOperators.AddOperator(PostfixIncrement, "++", Postfix, Unary, Token::Increment, 2, Left);
		m_DefinedOperators.AddOperator(PostfixDecrement, "--", Postfix, Unary, Token::Decrement, 2, Left);
		m_DefinedOperators.AddOperator(Call, "()", Postfix, Unary, Token::LeftParentheses, 2, Left);
		m_DefinedOperators.AddOperator(Subscript, "[]", Postfix, Unary, Token::LeftSquareBracket, 2, Left);
		m_DefinedOperators.AddOperator(MemberAccess, ".", Midfix, Unary, Token::MemberAccessor, 2, Left);

		// p = 3
		m_DefinedOperators.AddOperator(Closure, "closure", Prefix, Unary, Token::Closure, 3, Right);

		m_DefinedOperators.AddOperator(PrefixIncrement, "++", Prefix, Unary, Token::Increment, 3, Right);
		m_DefinedOperators.AddOperator(PrefixDecrement, "--", Prefix, Unary, Token::Decrement, 3, Right);
		m_DefinedOperators.AddOperator(UnaryPlus, "+", Prefix, Unary, Token::Add, 3, Right);
		m_DefinedOperators.AddOperator(UnaryMinus, "-", Prefix, Unary, Token::Subtract, 3, Right);
		m_DefinedOperators.AddOperator(LogicalNot, "!", Prefix, Unary, Token::Not, 3, Right);
		//m_DefinedOperators.AddOperator(BitwiseNot, Unary, Token::Not, 3, Left);
		//m_DefinedOperators.AddOperator(LogicalNot, Unary, Token::Not, 3, Left);

		// p = 5
		m_DefinedOperators.AddOperator(Multiplication, "*", Midfix, Binary, Token::Multiply, 5, Left);
		m_DefinedOperators.AddOperator(Division, "/", Midfix, Binary, Token::Divide, 5, Left);
		m_DefinedOperators.AddOperator(Remainder, "%", Midfix, Binary, Token::Remainder, 5, Left);

		// p = 6
		m_DefinedOperators.AddOperator(Addition, "+", Midfix, Binary, Token::Add, 6, Left);
		m_DefinedOperators.AddOperator(Subtraction, "-", Midfix, Binary, Token::Subtract, 6, Left);

		// p = 9
		m_DefinedOperators.AddOperator(LessThan, "<", Midfix, Binary, Token::LessThan, 9, Left);
		m_DefinedOperators.AddOperator(LessThanOrEqual, "<=", Midfix, Binary, Token::LessThanOrEqual, 9, Left);
		m_DefinedOperators.AddOperator(GreaterThan, ">", Midfix, Binary, Token::GreaterThan, 9, Left);
		m_DefinedOperators.AddOperator(GreaterThanOrEqual, ">=", Midfix, Binary, Token::GreaterThanOrEqual, 9, Left);

		// p = 10
		m_DefinedOperators.AddOperator(Equality, "==", Midfix, Binary, Token::Equality, 10, Left);
		m_DefinedOperators.AddOperator(NotEqual, "!=", Midfix, Binary, Token::NotEqual, 10, Left);

		// p = 16
		m_DefinedOperators.AddOperator(DirectAssignment, "=", Midfix, Binary, Token::SetEquals, 16, Right);
		m_DefinedOperators.AddOperator(CompoundAssignmentSum, "+=", Midfix, Binary, Token::PlusEquals, 16, Right);
		m_DefinedOperators.AddOperator(CompoundAssignmentDifference, "-=", Midfix, Binary, Token::MinusEquals, 16, Right);

		// p = 17
		m_DefinedOperators.AddOperator(Lambda, "=>", Midfix, Binary, Token::RightArrow, 17, Right);


		// Type modifier operators
		m_DefinedTypeModifierOperators.AddOperator(Subscript, "[]", Postfix, Unary, Token::LeftSquareBracket, 2, Right);
		m_DefinedTypeModifierOperators.AddOperator(Nullable, "?", Postfix, Unary, Token::QuestionMark, 1, Right);

		// Parselets for variables and literals
		m_PrefixParselets[Token::Identifier] = new IdentifierParselet();
		m_PrefixParselets[Token::IntLiteral] = new LiteralParselet();
		m_PrefixParselets[Token::DoubleLiteral] = new LiteralParselet();
		m_PrefixParselets[Token::FloatLiteral] = new LiteralParselet();
		m_PrefixParselets[Token::BoolLiteral] = new LiteralParselet();
		m_PrefixParselets[Token::StringLiteral] = new LiteralParselet();

		m_PrefixParselets[Token::LeftParentheses] = new ParenthesesGroupParselet();

		// Unary prefix operators. +a, -a
		m_PrefixParselets[Token::Increment] = new PrefixOperatorParselet();
		m_PrefixParselets[Token::Decrement] = new PrefixOperatorParselet();
		m_PrefixParselets[Token::Add] = new PrefixOperatorParselet();
		m_PrefixParselets[Token::Subtract] = new PrefixOperatorParselet();

		m_PrefixParselets[Token::Not] = new PrefixOperatorParselet();

		m_PrefixParselets[Token::Loop] = new LoopParselet();
		m_PrefixParselets[Token::Closure] = new ClosureParselet();


		// Infix. Binary operators, a + b etc.
		m_InfixParselets[Token::LeftParentheses] = new CallParselet();

		m_InfixParselets[Token::Multiply] = new BinaryOperatorParselet();
		m_InfixParselets[Token::Divide] = new BinaryOperatorParselet();
		m_InfixParselets[Token::Add] = new BinaryOperatorParselet();
		m_InfixParselets[Token::Subtract] = new BinaryOperatorParselet();

		m_InfixParselets[Token::LessThan] = new BinaryOperatorParselet();
		m_InfixParselets[Token::LessThanOrEqual] = new BinaryOperatorParselet();
		m_InfixParselets[Token::GreaterThan] = new BinaryOperatorParselet();
		m_InfixParselets[Token::GreaterThanOrEqual] = new BinaryOperatorParselet();

		m_InfixParselets[Token::Equality] = new BinaryOperatorParselet();
		m_InfixParselets[Token::SetEquals] = new BinaryOperatorParselet();

		m_InfixParselets[Token::PlusEquals] = new BinaryOperatorParselet();
		m_InfixParselets[Token::MinusEquals] = new BinaryOperatorParselet();

		m_InfixParselets[Token::RightArrow] = new LambdaParselet();

		// Unary postfix
		m_InfixParselets[Token::Increment] = new PostfixOperatorParselet();
		m_InfixParselets[Token::Decrement] = new PostfixOperatorParselet();


		// Type parselets
		m_PrefixTypeParselets[Token::LeftParentheses] = new ParenthesizedTypeParselet();
		m_PrefixTypeParselets[Token::Identifier] = new TypenameParselet();
		m_InfixTypeParselets[Token::LeftSquareBracket] = new ArrayTypeModifierParselet();
		m_InfixTypeParselets[Token::QuestionMark] = new NullableTypeModifierParselet();

		// Statements
		m_StatementParselets[Token::LeftCurlyBracket] = new BlockStatementParselet();

		m_StatementParselets[Token::Let] = new LetStatementParselet();
		//m_StatementParselets[Token::Identifier] = new LetStatementParselet();

		//m_StatementParselets[Token::LeftParentheses] = new TypenameStatementParselet();//(StatementParselet*)new ParenthesizedTypeParselet();

		m_StatementParselets[Token::While] = new ConditionalStatementParselet();
		m_StatementParselets[Token::If] = new ConditionalStatementParselet();
		m_StatementParselets[Token::For] = new ForStatementParselet();

		m_StatementParselets[Token::Continue] = new SingleKeywordParselet();
		m_StatementParselets[Token::Break] = new BreakParselet();
		m_StatementParselets[Token::Return] = new ReturnParselet();

		m_StatementParselets[Token::ClassKeyword] = new ClassDefinitionParselet();
	}

	Node* Parser::ParseProgram()
	{
		Program* program = new Program();

		while (true)
		{
			Token token = PeekToken(0);
			if (token.m_Type == Token::EndOfFile)
				break;

			Node* line = Parse();
			if (HasError()) return nullptr;

			if (!line) return nullptr; // handle

			program->m_Lines.push_back(line);
		}

		return program;
	}

	Node* Parser::Parse()
	{
		Token token = PeekToken(0);

		// Empty statement
		if (MatchToken(Token::Semicolon))
			return new Node();

		bool shouldParseAsExpression = m_StatementParselets.count(token.m_Type) == 0;

		// If a token is a "real" identifier (so not a typename), then also parse as expression
		if (TokenIsIdentifier(token))
			shouldParseAsExpression = true;

		// If an expression with parentheses that is not a type


		if (token.m_Type == Token::LeftParentheses)
		{
			int i = 1;
			// If the first non-'(' token we find is a typename
			while (true)
			{
				Token next = PeekToken(i);
				if (next.m_Type != Token::LeftParentheses)
				{
					if (!TokenIsTypename(next) && next.m_Type != Token::RightParentheses && next.m_Type != Token::Comma && next.m_Type != Token::RightArrow)
					{
						shouldParseAsExpression = true;
						break;
					}

					break;
				}
				i++;
			}
		}
		

		// If no statement parselets, then try to parse it as an expression
		if (shouldParseAsExpression)
		{
			Node* node = ParseExpression();
			if (HasError()) return nullptr;

			ConsumeToken(Token::Semicolon);
			if (HasError()) return nullptr;

			if (!node)
				return MakeError("Unexpected token " + token.TypeToString() + " in statement parser");

			return node;
		}

		token = ConsumeToken();

		StatementParselet* parselet = m_StatementParselets.at(token.m_Type);
		return parselet->Parse(*this, token);
	}

	// Inspired by https://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/
	Node* Parser::ParseExpression(int precedence)
	{
		if (HasError())
			return nullptr;

		// Parse prefix operators
		if (MatchToken(Token::Types::EndOfFile))
			return nullptr;

		if (PeekToken(0).m_Type == Token::Semicolon)
			return nullptr;

		Token token = ConsumeToken();
		if (m_PrefixParselets.count(token.m_Type) == 0)
		{
			// If the operator is a infix, then error
			if (m_InfixParselets.count(token.m_Type) != 0)
				return MakeError("Infix operator used as prefix");

			return MakeError("Unexpected token " + token.m_Value + " in expression, could not parse");
		}

		PrefixParselet* prefix = m_PrefixParselets.at(token.m_Type);
		Node* left = prefix->Parse(*this, token);
		if (HasError()) return nullptr;

		if (left->m_Type == NodeKind::BasicType)
			return MakeError("Cannot have typename " + left->ToString() + " in an expression");

		// Parse infix operators, such as normal binary operators or postfix unary operators (like a++)
		while (!HasError() && PeekToken(0).m_Type != Token::Types::EndOfFile && precedence < GetPrecedenceOfCurrentToken())
		{
			token = PeekToken(0);
			if (token.m_Type == Token::Types::EndOfFile)
				return left;

			token = ConsumeToken();

			// If no infix, then return the parsed prefix 
			if (m_InfixParselets.count(token.m_Type) == 0)
			{
				// If the operator is a prefix, then error
				if (m_PrefixParselets.count(token.m_Type) != 0)
					return MakeError("Prefix operator used as infix");

				return left;
			}

			InfixParselet* infix = m_InfixParselets.at(token.m_Type);

			left = infix->Parse(*this, left, token);
		}
		
		return left;
	}

	Type* Parser::ParseType(int precedence)
	{
		if (HasError())
			return nullptr;

		if (MatchToken(Token::Types::EndOfFile))
			return nullptr;

		// Animal?
		// (int, int)
		// (Animal?)[][][]

		// (((Animal[])[])[])

		// Parse prefix operators
		Token token = ConsumeToken();
		return ParseType(token, precedence);
	}

	Type* Parser::ParseType(Token token, int precedence)
	{
		if (m_PrefixTypeParselets.count(token.m_Type) == 0)
		{
			// If the operator is a infix, then error
			if (m_InfixTypeParselets.count(token.m_Type) != 0)
			{
				MakeError("Infix operator used as prefix");
				return nullptr;
			}

			MakeError("Unexpected token " + token.m_Value + " in type, could not parse");
			return nullptr;
		}

		PrefixTypeParselet* prefix = m_PrefixTypeParselets.at(token.m_Type);
		Type* left = (Type*)prefix->Parse(*this, token);
		if (HasError()) return nullptr;

		// Parse infix operators, such as normal binary operators or postfix unary operators (like a++)
		while (!HasError() && PeekToken(0).m_Type != Token::Types::EndOfFile && precedence < GetPrecedenceOfCurrentTokenType())
		{
			token = PeekToken(0);
			if (token.m_Type == Token::Types::EndOfFile)
				return nullptr;

			token = ConsumeToken();

			// If no infix, then return the parsed prefix 
			if (m_InfixTypeParselets.count(token.m_Type) == 0)
			{
				// If the operator is a prefix, then error
				if (m_PrefixTypeParselets.count(token.m_Type) != 0)
				{
					MakeError("Prefix operator used as infix");
					return nullptr;
				}

				return nullptr;
			}

			InfixTypeParselet* infix = m_InfixTypeParselets.at(token.m_Type);

			left = (Type*)infix->Parse(*this, left, token);
		}

		return left;
	}

	Type* Parser::ParseTypeAnnotation()
	{
		if (PeekToken().m_Type != Token::Colon)
			return nullptr;

		ConsumeToken(Token::Colon);
		if (HasError()) return nullptr;

		return ParseType(ConsumeToken());
	}

	Identifier* Parser::ParseIdentifier(Token token)
	{
		if (!TokenIsIdentifier(token))
		{
			MakeError("Expected identifier, but got '" + token.ToFormattedValueString() + "'", token);
			return nullptr;
		}

		std::string variableName = token.m_Value;

		return new Identifier(variableName);
	}

	TupleExpression* Parser::ParseTupleExpression(Token token)
	{
		std::vector<Node*> elements;

		// Parse until we find a closing parentheses
		if (!MatchToken(Token::RightParentheses))
		{
			do
			{
				Node* node = ParseExpression();
				if (!node || HasError())
					return {};

				elements.push_back(node);
			} while (MatchToken(Token::Comma));

			ConsumeToken(Token::RightParentheses);
		}

		return new TupleExpression(elements);
	}

	VariableDeclaration* Parser::ParseVariableDeclaration(Token token, bool consumeEndToken, std::vector<Token::Types> endTokens)
	{
		Identifier* name = ParseIdentifier(token);
		if (HasError()) return nullptr;

		Type* type = ParseTypeAnnotation();

		// a:int;
		if (consumeEndToken)
		{
			for (auto& tokenType : endTokens)
			{
				if (MatchToken(tokenType))
					return new VariableDeclaration(name, type, nullptr);
			}
		}
		else
		{
			for (auto& tokenType : endTokens)
			{
				if (MatchTokenNoConsume(tokenType))
					return new VariableDeclaration(name, type, nullptr);
			}
		}

		// a:int = ...;
		ConsumeToken(Token::SetEquals);
		if (HasError()) return nullptr;

		Node* assignedValue = ParseExpression();
		if (HasError()) return nullptr;

		if (!assignedValue)
		{
			MakeError("Expected expression on right hand side of assignment, but got " +
				PeekToken(0).ToFormattedValueString(), PeekToken(0));
			return nullptr;
		}

		if (HasError()) return nullptr;

		if (consumeEndToken)
		{
			// Look if the next token is one of the end tokens
			// if so, consume it and return
			for (auto& tokenType : endTokens)
			{
				if (MatchToken(tokenType))
					break;
			}
		}

		return new VariableDeclaration(name, type, assignedValue);
	}

	FunctionParameters* Parser::ParseFunctionParameters(Token token)
	{
		std::vector<VariableDeclaration*> parameters;

		// Parse until we find a closing parentheses
		if (!MatchToken(Token::RightParentheses))
		{
			do
			{
				std::vector<Token::Types> endTokens = { Token::Comma, Token::RightParentheses };
				VariableDeclaration* declaration = ParseVariableDeclaration(ConsumeToken(), false /* dont consume end token */, endTokens);
				if (HasError())
					return nullptr;

				parameters.push_back(declaration);
			} while (MatchToken(Token::Comma));

			ConsumeToken(Token::RightParentheses);
		}

		return new FunctionParameters(parameters);
	}

	float Parser::TemporaryEvaluator(Node* node)
	{
		switch (node->m_Type)
		{
		case NodeKind::Program:
		case NodeKind::BlockStatement:
		{
			Scope* scope = (Scope*)node;

			float result = -1;
			for (auto& line : scope->m_Lines)
			{
				result = TemporaryEvaluator(line);
			}
			return result;
		}
			break;
		case NodeKind::Identifier:
			break;
		case NodeKind::VariableDeclaration:
			break;
		case NodeKind::BinaryExpression:
		{
			BinaryExpression* expr = (BinaryExpression*)node;
			float lhs = TemporaryEvaluator(expr->m_Lhs);
			float rhs = TemporaryEvaluator(expr->m_Rhs);

			switch (expr->m_Operator.m_Name)
			{
			case Operators::Name::Addition:
				return lhs + rhs;
			case Operators::Name::Subtraction:
				return lhs - rhs;
			case Operators::Name::Multiplication:
				return lhs * rhs;
			case Operators::Name::Division:
				return (float)lhs / (float)rhs;
			default:
				abort();
			}
		}
		case NodeKind::UnaryExpression:
		{
			UnaryExpression* expr = (UnaryExpression*)node;
			float arg = TemporaryEvaluator(expr->m_Operand);

			switch (expr->m_Operator.m_Name)
			{
			case Operators::Name::UnaryMinus:
				return -arg;
			case Operators::Name::UnaryPlus:
				return +arg;
			default:
				abort();
			}
		}
		
			break;
		case NodeKind::IntLiteral:
			return float(((IntLiteral*)node)->m_Value);
			break;
		case NodeKind::FloatLiteral:
			break;
		case NodeKind::DoubleLiteral:
			break;
		case NodeKind::StringLiteral:
			break;
		default:
			break;
		}
	}

	Node* Parser::MakeError(const std::string& message, Token errorToken, CompileTimeError::Severity severity)
	{
		MakeError_Void(message, errorToken, severity);
		return nullptr;
	}

	Node* Parser::MakeError(const std::string& message, CompileTimeError::Severity severity)
	{
		MakeError_Void(message, m_LastConsumedToken, severity);
		return nullptr;
	}

	Token Parser::ConsumeToken()
	{
		Token first = m_TokenStream.front();

		m_TokenStream.pop_front();
		m_LastConsumedToken = first;

		return first;
	}

	std::optional<Token> Parser::ConsumeToken(Token::Types expectedType)
	{
		Token next = PeekToken(0);
		if (next.m_Type != expectedType)
		{
			MakeError("Expected " + TokenTypeToString(expectedType) + ", found '" + next.ToFormattedValueString() + "'", next);
			return {};
		}

		return ConsumeToken();
	}

	Token Parser::PeekToken(int distance)
	{
		return m_TokenStream[distance];
	}

	bool Parser::MatchToken(Token::Types expectedType)
	{
		Token token = PeekToken(0);
		if (token.m_Type != expectedType)
			return false;

		ConsumeToken();
		return true;
	}
	bool Parser::MatchTokenNoConsume(int peekDistance, Token::Types expectedType)
	{
		Token next = PeekToken(peekDistance);
		if (next.m_Type != expectedType)
			return false;

		return true;
	}

	bool Parser::EnsureToken(int peekDistance, Token::Types expectedType)
	{
		Token next = PeekToken(peekDistance);
		if (next.m_Type != expectedType)
		{
			MakeError("Expected " + TokenTypeToString(expectedType) + ", found '" + next.ToFormattedValueString() + "'", next);
			return false;
		}

		return true;
	}

	int Parser::GetPrecedenceOfCurrentToken()
	{
		auto op = m_DefinedOperators.GetAny(PeekToken(0).m_Type);

		// Default value if no precedence exists
		if (!op.has_value()) 
			return 0;

		return op.value().GetParsePrecedence();
	}

	int Parser::GetPrecedenceOfCurrentTokenType()
	{
		auto op = m_DefinedTypeModifierOperators.GetAny(PeekToken(0).m_Type);

		// Default value if no precedence exists
		if (!op.has_value())
			return 0;

		return op.value().GetParsePrecedence();
	}
};