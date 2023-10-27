#include "Parser.h"

#include <iostream>

#include "../Utils.hpp"

#define RETURN_IF_ERROR() if (HasError()) return nullptr;

namespace Ö::AST
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



		// Parselets for variables and literals
		m_PrefixParselets[Token::Identifier] = new IdentifierParselet();
		m_PrefixParselets[Token::IntLiteral] = new LiteralParselet();
		m_PrefixParselets[Token::DoubleLiteral] = new LiteralParselet();
		m_PrefixParselets[Token::FloatLiteral] = new LiteralParselet();
		m_PrefixParselets[Token::StringLiteral] = new LiteralParselet();

		m_PrefixParselets[Token::LeftParentheses] = new ParenthesesGroupParselet();

		// Unary prefix operators. +a, -a
		m_PrefixParselets[Token::Increment] = new PrefixOperatorParselet();
		m_PrefixParselets[Token::Decrement] = new PrefixOperatorParselet();
		m_PrefixParselets[Token::Add] = new PrefixOperatorParselet();
		m_PrefixParselets[Token::Subtract] = new PrefixOperatorParselet();

		m_PrefixParselets[Token::Not] = new PrefixOperatorParselet();

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

		// Unary postfix
		m_InfixParselets[Token::Increment] = new PostfixOperatorParselet();
		m_InfixParselets[Token::Decrement] = new PostfixOperatorParselet();
	}

	Node Parser::CreateRootNode()
	{
		Node node(nullptr);
		node.m_Type = NodeType::Root;
		
		return node;
	}

	// Inspired by https://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/
	Node* Parser::ParseExpression(int precedence)
	{
		if (HasError())
			return nullptr;

		// Parse prefix operators
		Token token = ConsumeToken();
		if (token.m_Type == Token::Types::EndOfFile)
			return nullptr;

		if (m_PrefixParselets.count(token.m_Type) == 0)
			return MakeError("Unexpected token " + token.m_Value + ", could not parse");

		PrefixParselet* prefix = m_PrefixParselets.at(token.m_Type);
		Node* left = prefix->Parse(*this, token, nullptr);

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

			left = infix->Parse(*this, left, token, nullptr);
		}
		
		return left;
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

	//Node* Parser::ParseScope(Tokens& tokens, Node* parent)
	//{
	//	if (tokens[0].m_Type == Token::LeftCurlyBracket || parent->m_Type == NodeType::Root)
	//	{
	//		LinesOfTokens lines;
	//		Scope* scope;

	//		// There are no first and last brackets to exlude if it's the root program, so iterate all tokens
	//		if (parent->m_Type == NodeType::Root)
	//		{
	//			lines = MakeScopeIntoLines(tokens, 0, tokens.size(), 0);
	//			scope = new Program(parent);
	//		}
	//		else
	//		{
	//			lines = MakeScopeIntoLines(tokens, 1, tokens.size() - 1, tokens[0].m_Depth);
	//			scope = new BlockStatement(parent);
	//		}

	//		// Finally, evaluate all lines
	//		for (auto& line : lines)
	//		{
	//			Node* statement = CreateAST(line, scope);
	//			RETURN_IF_ERROR();

	//			scope->m_Lines.push_back(statement);
	//		}

	//		return scope;
	//	}

	//	return nullptr;
	//}


	bool Parser::IsInsideBrackets(Tokens tokens, int start)
	{
		for (int i = start; i >= 0; i--) // Walk backwards from here
		{
			if (tokens[i].m_Type == Token::LeftParentheses ||
				tokens[i].m_Type == Token::LeftSquareBracket) {
				if (tokens[i].m_Depth == tokens[start].m_Depth)
					return true;
			}
		}

		for (int i = start; i < tokens.size(); i++) // Walk forwards from here
		{
			if (tokens[i].m_Type == Token::RightParentheses ||
				tokens[i].m_Type == Token::RightSquareBracket)
			{
				assert(tokens[i].m_Depth != -1);

				if (tokens[i].m_Depth == tokens[start].m_Depth)
					return true;
			}
		}

		return false;
	}

	std::optional<int> Parser::FindMatchingEndBracket(Tokens& tokens, Token& startToken)
	{
		Token::Types typeOfEnd = Token::Empty;
		if (startToken.m_Type == Token::LeftParentheses)
			typeOfEnd = Token::RightParentheses;
		else if (startToken.m_Type == Token::LeftCurlyBracket)
			typeOfEnd = Token::RightCurlyBracket;
		else if (startToken.m_Type == Token::LeftSquareBracket)
			typeOfEnd = Token::RightSquareBracket;

		for (int i = 0; i < tokens.size(); i++)
		{
			if (tokens[i].m_Depth == startToken.m_Depth && tokens[i].m_Type == typeOfEnd)
				return i;
		}

		return std::nullopt;
	}
	std::optional<int> Parser::FindMatchingStartBracket(Tokens& tokens, Token& endToken)
	{
		Token::Types typeOfEnd = Token::Empty;
		if (endToken.m_Type == Token::RightParentheses)
			typeOfEnd = Token::LeftParentheses;
		else if (endToken.m_Type == Token::RightCurlyBracket)
			typeOfEnd = Token::LeftCurlyBracket;
		else if (endToken.m_Type == Token::RightSquareBracket)
			typeOfEnd = Token::LeftSquareBracket;

		for (int i = tokens.size() - 1; i >= 0; i--)
		{
			if (tokens[i].m_Depth == endToken.m_Depth && tokens[i].m_Type == typeOfEnd)
				return i;
		}

		return std::nullopt;
	}

	Node* Parser::MakeError(const std::string& message)
	{
		m_Error = message;
		return nullptr;
	}
	void Parser::MakeErrorVoid(const std::string& message)
	{
		m_Error = message;
	}

	Token Parser::ConsumeToken()
	{
		Token first = m_TokenStream.front();

		m_TokenStream.pop_front();

		return first;
	}

	std::optional<Token> Parser::ConsumeToken(Token::Types expectedType)
	{
		Token next = PeekToken(0);
		if (next.m_Type != expectedType)
		{
			MakeError("Expected token " + Token(expectedType, 0).ToString() + " but got " + next.ToString());
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

	int Parser::GetPrecedenceOfCurrentToken()
	{
		auto op = m_DefinedOperators.GetAny(PeekToken(0).m_Type);

		// Default value if no precedence exists
		if (!op.has_value()) 
			return 0;

		return op.value().GetParsePrecedence();
	}

	// Parslets
	// Identifers and literals

	Node* IdentifierParselet::Parse(Parser& parser, Token token, Node* parent)
	{
		return new Identifier(parent, token.m_Value);
	}
	Node* LiteralParselet::Parse(Parser& parser, Token token, Node* parent)
	{
		Token nextToken = parser.PeekToken(0);
		if (nextToken.IsLiteral())
			return parser.MakeError("Cannot have two literals next to each other");

		if (token.m_Type == Token::IntLiteral)
			return new IntLiteral(parent, std::stoi(token.m_Value));
		if (token.m_Type == Token::DoubleLiteral)
			return new DoubleLiteral(parent, StringToDouble(token.m_Value));
		if (token.m_Type == Token::FloatLiteral)
			abort();
		if (token.m_Type == Token::StringLiteral)
			return new StringLiteral(parent, token.m_Value);

		abort();
		return nullptr;
	}
	Node* ParenthesesGroupParselet::Parse(Parser& parser, Token token, Node* parent)
	{
		Node* expression = parser.ParseExpression();

		auto result = parser.ConsumeToken(Token::Types::RightParentheses);
		if (!result.has_value()) 
			return nullptr;

		return expression;
	}

	// Operators

	Node* PrefixOperatorParselet::Parse(Parser& parser, Token token, Node* parent)
	{
		auto opOp = parser.m_DefinedOperators.GetUnaryPrefix(token.m_Type);
		assert(opOp.has_value());
		auto& op = opOp.value();

		//Token nextToken = parser.PeekToken(0);
		//if (nextToken.IsOperator())
			//return parser.MakeError("Cannot have two binary operators next to each other");

		Node* operand = parser.ParseExpression(op.GetParsePrecedence());

		if (parser.HasError())
			return nullptr;

		if (!operand)
			return parser.MakeError("Expected expression next to " + op.ToString());

		return new UnaryExpression(parent, operand, op);
	}

	Node* PostfixOperatorParselet::Parse(Parser& parser, Node* left, Token token, Node* parent)
	{
		auto opOp = parser.m_DefinedOperators.GetUnaryPostfix(token.m_Type);
		assert(opOp.has_value());
		auto& op = opOp.value();

		return new UnaryExpression(parent, left, op);
	}

	Node* BinaryOperatorParselet::Parse(Parser& parser, Node* left, Token token, Node* parent)
	{
		auto opOp = parser.m_DefinedOperators.GetBinary(token.m_Type);
		assert(opOp.has_value());
		auto& op = opOp.value();

		Token nextToken = parser.PeekToken(0);
		if (nextToken.IsOperator())
			return parser.MakeError("Cannot have two binary operators next to each other");

		Node* right = parser.ParseExpression(op.GetParsePrecedence());

		if (parser.HasError())
			return nullptr;

		if (!right)
			return parser.MakeError("Expected expression on right side of " + op.ToString());

		return new BinaryExpression(parent, left, op, right);
	}

	Node* CallParselet::Parse(Parser& parser, Node* left, Token token, Node* parent)
	{
		std::vector<Node*> arguments;
		
		// Parse until we find a closing parentheses
		if (!parser.MatchToken(Token::RightParentheses))
		{
			do
			{
				arguments.push_back(parser.ParseExpression());
			} while (parser.MatchToken(Token::Comma));
			parser.ConsumeToken(Token::RightParentheses);
		}

		return new CallExpression(nullptr, left, arguments);
	}
};