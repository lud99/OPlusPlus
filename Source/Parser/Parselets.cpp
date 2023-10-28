#include "Parselets.h"

#include <sstream>

static double StringToDouble(std::string s)
{
	std::istringstream streamToConvert(s);
	streamToConvert.imbue(std::locale("C"));
	double result = 0.0;
	streamToConvert >> result;

	if (streamToConvert.fail() || streamToConvert.bad() || !streamToConvert.eof()) {
		abort();
	}

	return result;
}

namespace Ö::AST
{
	// Identifers and literals

	Node* IdentifierParselet::Parse(Parser& parser, Token token)
	{
		// Check if the identifier is a typename
		if (parser.m_TypeTable.HasType(token.m_Value))
			return new Type(token.m_Value);
		else
			return new Identifier(token.m_Value);
	}
	Node* LiteralParselet::Parse(Parser& parser, Token token)
	{
		Token nextToken = parser.PeekToken(0);
		if (nextToken.IsLiteral())
			return parser.MakeError("Cannot have two literals next to each other");

		if (token.m_Type == Token::IntLiteral)
			return new IntLiteral(std::stoi(token.m_Value));
		if (token.m_Type == Token::DoubleLiteral)
			return new DoubleLiteral(StringToDouble(token.m_Value));
		if (token.m_Type == Token::FloatLiteral)
			abort();
		if (token.m_Type == Token::StringLiteral)
			return new StringLiteral(token.m_Value);

		abort();
		return nullptr;
	}
	Node* ParenthesesGroupParselet::Parse(Parser& parser, Token token)
	{
		Node* expression = parser.ParseExpression();

		auto result = parser.ConsumeToken(Token::Types::RightParentheses);
		if (!result.has_value())
			return nullptr;

		return expression;
	}

	// Operators

	Node* PrefixOperatorParselet::Parse(Parser& parser, Token token)
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

		return new UnaryExpression(operand, op);
	}

	Node* PostfixOperatorParselet::Parse(Parser& parser, Node* left, Token token)
	{
		auto opOp = parser.m_DefinedOperators.GetUnaryPostfix(token.m_Type);
		assert(opOp.has_value());
		auto& op = opOp.value();

		return new UnaryExpression(left, op);
	}

	Node* BinaryOperatorParselet::Parse(Parser& parser, Node* left, Token token)
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

		return new BinaryExpression(left, op, right);
	}

	Node* CallParselet::Parse(Parser& parser, Node* left, Token token)
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

		return new CallExpression(left, arguments);
	}

	bool IsValidElseBody(Node* body)
	{
		if (!body) return false;
		return body->m_Type == NodeType::BlockStatement
			|| body->m_Type == NodeType::IfStatement
			|| body->m_Type == NodeType::WhileStatement
			|| body->m_Type == NodeType::BlockStatement;
	}

	Node* ConditionalStatementParselet::Parse(Parser& parser, Token token)
	{
		parser.ConsumeToken(Token::LeftParentheses);
		if (parser.HasError()) return nullptr;

		Node* condition = parser.ParseExpression();

		auto result = parser.ConsumeToken(Token::RightParentheses);
		if (!result.has_value())
			return parser.MakeError("Found no closing parentheses for conditional statement");

		Node* body = parser.Parse();
		if (!body || body->m_Type != NodeType::BlockStatement)
			return parser.MakeError("Expected left curly bracket after " + token.ToString() + " statement");

		if (token.m_Type == Token::While)
			return (Node*) new WhileStatement(condition, (BlockStatement*)body);

		assert(token.m_Type == Token::If);

		// If no else
		if (!parser.MatchToken(Token::Else))
			return (Node*) new IfStatement(condition, (BlockStatement*)body, nullptr);

		// Try to parse for else
		Node* elseArm = parser.Parse();
		if (parser.HasError()) return nullptr;

		if (!IsValidElseBody(elseArm))
			return parser.MakeError("Unexpected token after 'else' in if statement");

		return (Node*) new IfStatement(condition, (BlockStatement*)body, (BlockStatement*)elseArm);
	}

	Node* BlockStatementParselet::Parse(Parser& parser, Token token)
	{
		BlockStatement* blockNode = new BlockStatement();

		while (true)
		{
			token = parser.PeekToken(0);
			if (token.m_Type == Token::RightCurlyBracket)
				break;

			if (token.m_Type == Token::EndOfFile)
				return parser.MakeError("No closing curly bracket");

			Node* line = parser.Parse();
			if (parser.HasError()) return nullptr;

			if (!line) return nullptr; // handle

			blockNode->m_Lines.push_back(line);
		}

		parser.ConsumeToken(Token::RightCurlyBracket);

		token = parser.PeekToken(0);
		if (token.m_Type == Token::EndOfFile)
			return blockNode;


		return blockNode;
	}
	Node* ForStatementParselet::Parse(Parser& parser, Token token)
	{
		parser.ConsumeToken(Token::LeftParentheses);
		if (parser.HasError()) return nullptr;

		// for (initialization; condition; advancement) {...}

		Node* initialization = parser.Parse();
		//parser.ConsumeToken(Token::Semicolon);
		if (parser.HasError()) return nullptr;

		Node* condition = parser.ParseExpression();
		parser.ConsumeToken(Token::Semicolon);
		if (parser.HasError()) return nullptr;

		// If the next token is ')', then there is no advancement expression
		Node* advancement = nullptr;
		if (parser.PeekToken(0).m_Type != Token::RightParentheses)
		{
			Node* advancement = parser.ParseExpression();
			if (parser.HasError()) return nullptr;
		}

		auto result = parser.ConsumeToken(Token::RightParentheses);
		if (!result.has_value())
			return parser.MakeError("Found no closing parentheses for conditional statement");

		Node* body = parser.Parse();
		if (!body || body->m_Type != NodeType::BlockStatement)
			return parser.MakeError("Expected left curly bracket after " + token.ToString() + " statement");

		return (Node*) new ForStatement(initialization, condition, advancement, (BlockStatement*)body);
	}

	Node* SingleKeywordParselet::Parse(Parser& parser, Token token)
	{
		parser.ConsumeToken(Token::Semicolon);
		if (parser.HasError()) return nullptr;

		NodeType type;
		if (token.m_Type == Token::Break)
			type = NodeType::Break;
		else if (token.m_Type == Token::Continue)
			type = NodeType::Continue;
		else
			abort();

		return (Node*) new SingleKeywordStatement(type);
	}
	Node* ReturnParselet::Parse(Parser& parser, Token token)
	{
		Node* returnValue = parser.ParseExpression();
		parser.ConsumeToken(Token::Semicolon);
		if (parser.HasError()) return nullptr;

		return (Node*) new ReturnStatement(returnValue);
	}

	Node* ParseVariableDeclaration(Parser& parser, Token token, Type* type, Identifier* name, Token::Types endToken = Token::Semicolon)
	{
		if (parser.MatchTokenNoConsume(endToken))
			return new VariableDeclaration(type, name, nullptr);

		parser.ConsumeToken(Token::SetEquals);
		if (parser.HasError()) return nullptr;

		Node* assignedValue = parser.ParseExpression();
		if (parser.HasError()) return nullptr;

		//parser.ConsumeToken(endToken);

		return new VariableDeclaration(type, name, assignedValue);
	}

	std::tuple<Type*, Identifier*> ParseTypenameAndName(Parser& parser, Token token)
	{
		std::string variableType = token.m_Value;

		token = parser.PeekToken(0);

		if (parser.TokenIsTypename(token))
		{
			parser.MakeErrorVoid("Cannot have two types next to each other in statement");
			return {};
		}

		// todo: allow scope resolution (::)
		parser.ConsumeToken(Token::Identifier);
		if (parser.HasError()) return {};

		std::string variableName = token.m_Value;

		return std::make_tuple(new Type(variableType), new Identifier(variableName));
	}

	std::vector<Node*> ParseFunctionPrototypeParameters(Parser& parser, Token token)
	{
		std::vector<Node*> parameters;

		// Parse until we find a closing parentheses
		if (!parser.MatchToken(Token::RightParentheses))
		{
			do
			{
				token = parser.ConsumeToken();

				auto [type, name] = ParseTypenameAndName(parser, token);
				if (parser.HasError()) return {};

				Token::Types endToken = Token::Comma;
				token = parser.PeekToken(0);
				if (token.m_Type == Token::RightParentheses)
					endToken = Token::RightParentheses;

				Node* variableDeclaration = ParseVariableDeclaration(parser, token, type, name, endToken);

				parameters.push_back(variableDeclaration);
			} while (parser.MatchToken(Token::Comma));

			parser.ConsumeToken(Token::RightParentheses);
		}

		return parameters;
	}

	Node* ParseFunctionDefinition(Parser& parser, Token token, Type* returnType, Identifier* name)
	{
		std::vector<Node*> parameters = ParseFunctionPrototypeParameters(parser, token);

		return new FunctionPrototypeStatement(returnType, name, parameters);
	}

	Node* TypenameStatementParselet::Parse(Parser& parser, Token token)
	{
		// The statement could now be either a variable declaration, or a function declaration
		// int a; int a = ...; int a(...); int a (...) {...}

		auto [type, name] = ParseTypenameAndName(parser, token);
		if (parser.HasError()) return nullptr;

		// If function
		if (parser.MatchToken(Token::LeftParentheses))
			return ParseFunctionDefinition(parser, token, type, name);

		// Otherwise it's a variable declaration (int a; or int a = ...;)
		// int a;
		if (parser.MatchToken(Token::Semicolon))
			return new VariableDeclaration(type, name, nullptr);

		// int a = ...;
		parser.ConsumeToken(Token::SetEquals);
		if (parser.HasError()) return nullptr;

		Node* assignedValue = parser.ParseExpression();
		if (parser.HasError()) return nullptr;

		parser.ConsumeToken(Token::Semicolon);

		return new VariableDeclaration(type, name, assignedValue);
	}
}