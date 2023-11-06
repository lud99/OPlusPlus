#include "Parser.h"

#include <iostream>

#include "Parselets.h"
#include "../Utils.hpp"

#define RETURN_IF_ERROR() if (HasError()) return nullptr;

namespace O::AST
{
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

		// Statements
		m_StatementParselets[Token::LeftCurlyBracket] = new BlockStatementParselet();

		m_StatementParselets[Token::Identifier] = new TypenameStatementParselet();

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

		// If no statement parselets, then try to parse it as an expression
		if (m_StatementParselets.count(token.m_Type) == 0 || (token.m_Type == Token::Identifier && TokenIsIdentifier(token)))
		{
			Node* node = ParseExpression();
			if (HasError()) return nullptr;

			ConsumeToken(Token::Semicolon);
			if (HasError()) return nullptr;

			if (!node)
				return MakeErrorButPretty("Unexpected token " + token.TypeToString() + " in statement parser");

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
				return MakeErrorButPretty("Infix operator used as prefix");

			return MakeErrorButPretty("Unexpected token " + token.m_Value + " in expression, could not parse");
		}

		PrefixParselet* prefix = m_PrefixParselets.at(token.m_Type);
		Node* left = prefix->Parse(*this, token);
		if (HasError()) return nullptr;

		if (left->m_Type == NodeType::Typename)
			return MakeErrorButPretty("Cannot have typename " + left->ToString() + " in an expression");

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
					return MakeErrorButPretty("Prefix operator used as infix");

				return left;
			}

			InfixParselet* infix = m_InfixParselets.at(token.m_Type);

			left = infix->Parse(*this, left, token);
		}
		
		return left;
	}

	std::tuple<Type*, Identifier*> Parser::ParseTypeAndName(Token token)
	{
		// parsing grouped type declaration, tuple or function prototype
		if (token.m_Type == Token::LeftParentheses)
		{
			// Look ahead for a comma inside the parantheses
			int i = 0;
			bool isTuple = false;
			while (true)
			{
				Token peekToken = PeekToken(i);

				if (peekToken.m_Type == Token::EndOfFile)
				{
					MakeErrorButPretty("No closing parentheses found", token);
					return {};
				}

				if (peekToken.m_Type == Token::LeftParentheses)
				{
					//ParseTypeAndName()
				}

				// If ')' is found before a comma, then it is not a tuple
				// But if it is the first token we peek, '() would it look like', then it is an empty tuple
				if (peekToken.m_Type == Token::RightParentheses)
				{
					// Looks like this: ()
					if (i == 0)
					{
						MakeErrorButPretty("() is not a type");
						return {};
					}

					if (isTuple)
					{
						// If it has the format '(...) =>' then it is a lambda
						//if (MatchTokenNoConsume(i + 1, Token::RightArrow))
						//	return ParseFunctionDefinition(token, nullptr, nullptr);

						//if (MatchTokenNoConsume(i + 1, Token::LeftCurlyBracket))
						//	return MakeErrorButPretty("Expected '=>' after lamda parameters, block scopes are not supported in lambda");

						//// Otherwise a normal tuple
						//return ParseTupleExpression();
					}

					// No tuple :(
					break;
				}

				// It is a tuple!
				if (peekToken.m_Type == Token::Comma)
					isTuple = true;

				i++;
			}
		}

		std::string variableType = token.m_Value;

		token = PeekToken(0);

		if (TokenIsTypename(token))
		{
			MakeErrorButPretty("Cannot have two types next to each other in statement", token);
			return {};
		}

		// todo: allow scope resolution (::), ?, [] etc 
		ConsumeToken(Token::Identifier);
		if (HasError()) return {};

		std::string variableName = token.m_Value;

		return std::make_tuple(new Type(variableType), new Identifier(variableName));
	}

	VariableDeclaration* Parser::ParseVariableDeclaration(Token token, Type* type, Identifier* name, bool consumeEndToken, Token::Types endToken)
	{
		// int a;
		if (consumeEndToken)
		{
			if (MatchToken(endToken))
				return new VariableDeclaration(type, name, nullptr);
		}
		else
		{
			if (MatchTokenNoConsume(endToken))
				return new VariableDeclaration(type, name, nullptr);
		}

		// int a = ...;
		ConsumeToken(Token::SetEquals);
		if (HasError()) return nullptr;

		Node* assignedValue = ParseExpression();
		if (HasError()) return nullptr;

		if (!assignedValue)
		{
			MakeErrorButPretty("Expected expression on right hand side of assignment, but got " +
				PeekToken(0).ToFormattedValueString(), PeekToken(0));
			return nullptr;
		}

		if (HasError()) return nullptr;

		if (consumeEndToken)
			ConsumeToken(endToken);

		return new VariableDeclaration(type, name, assignedValue);
	}

	std::vector<VariableDeclaration*> Parser::ParseFunctionParameters(Token token)
	{
		std::vector<VariableDeclaration*> parameters;

		// Parse until we find a closing parentheses
		if (!MatchToken(Token::RightParentheses))
		{
			do
			{
				token = ConsumeToken();

				auto [type, name] = ParseTypeAndName(token);
				if (HasError()) return {};

				Token::Types endToken = Token::Comma;
				token = PeekToken(0);
				if (token.m_Type == Token::RightParentheses)
					endToken = Token::RightParentheses;

				parameters.push_back(ParseVariableDeclaration(token, type, name, false /* dont consume comma */, endToken));
			} while (MatchToken(Token::Comma));

			ConsumeToken(Token::RightParentheses);
		}

		return parameters;
	}

	float Parser::TemporaryEvaluator(Node* node)
	{
		switch (node->m_Type)
		{
		case NodeType::Program:
		case NodeType::BlockStatement:
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
		case NodeType::Identifier:
			break;
		case NodeType::VariableDeclaration:
			break;
		case NodeType::AssignmentExpression:
			break;
		case NodeType::BinaryExpression:
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
		case NodeType::UnaryExpression:
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
		case NodeType::IntLiteral:
			return float(((IntLiteral*)node)->m_Value);
			break;
		case NodeType::FloatLiteral:
			break;
		case NodeType::DoubleLiteral:
			break;
		case NodeType::StringLiteral:
			break;
		default:
			break;
		}
	}

	Node* Parser::MakeErrorButPretty(const std::string& message, Token errorToken, ParserError::Severity severity)
	{
		ParserError error;
		error.message = message;
		error.severity = severity;
		error.token = errorToken;

		m_Errors.push_back(error);
		return nullptr;
	}
	Node* Parser::MakeErrorButPretty(const std::string& message, ParserError::Severity severity)
	{
		return MakeErrorButPretty(message, m_LastConsumedToken, severity);
	}

	void Parser::PrintErrors()
	{
		std::string source = O::Lexer::Lexer::ReconstructSourcecode(m_Tokens);
		auto sourceLines = split(source, '\n');

		for (auto& error : m_Errors)
		{
			auto startPosition = error.token.m_StartPosition;

			std::string severity = std::string(magic_enum::enum_name(error.severity));
			std::cout << severity << ": " << error.message << "\n\n";

			std::string lineOfError = sourceLines[startPosition.line];

			int indent = std::to_string(startPosition.line).length();

			std::string leftPadding = " " + Replicate(indent, " ") + " | ";

			if (startPosition.line != 0)
			{
				std::string lineBefore = sourceLines[startPosition.line - 1];
				std::cout << leftPadding << lineBefore << "\n";
			}
			std::cout << " " << startPosition.line + 1 << " | " << lineOfError << "\n";
			std::cout << leftPadding;

			int errorMarkerLength = std::max(error.token.ToFormattedValueString().length(), size_t(1));

			std::cout << Replicate(startPosition.column, " ");
			std::cout << Replicate(errorMarkerLength, "^");

			std::cout << "\n\n";

			//std::cout << error.positionInSource.line + 1 << ":" << error.positionInSource.column + 1;
		}
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
			MakeErrorButPretty("Expected " + TokenTypeToString(expectedType) + ", found '" + next.ToFormattedValueString() + "'", next);
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
			MakeErrorButPretty("Expected " + TokenTypeToString(expectedType) + ", found '" + next.ToFormattedValueString() + "'", next);
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
};