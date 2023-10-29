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
			return parser.MakeErrorButPretty("Cannot have two literals next to each other", nextToken);

		if (token.m_Type == Token::IntLiteral)
			return new IntLiteral(std::stoi(token.m_Value));
		if (token.m_Type == Token::DoubleLiteral)
			return new DoubleLiteral(StringToDouble(token.m_Value));
		if (token.m_Type == Token::FloatLiteral)
			abort();
		if (token.m_Type == Token::BoolLiteral)
			return new BoolLiteral(token.m_Value == "true" ? true : false);
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
			return parser.MakeErrorButPretty("Expected expression next to " + op.ToString(), token);

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
		{
			// Two ops next to each other is only allowed if {binary} {unary prefix}, eg 2 + !2
			if (!parser.m_DefinedOperators.GetUnaryPrefix(nextToken.m_Type))
				return parser.MakeErrorButPretty("Cannot have two binary operators next to each other", token);
		}

		Node* right = parser.ParseExpression(op.GetParsePrecedence());

		if (parser.HasError())
			return nullptr;

		if (!right)
			return parser.MakeErrorButPretty("Expected expression on right side of " + op.ToString(), nextToken);

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
			return parser.MakeErrorButPretty("Found no closing parentheses for conditional statement");

		Node* body = parser.Parse();
		if (!body || body->m_Type != NodeType::BlockStatement)
			return parser.MakeErrorButPretty("Expected left curly bracket after " + token.TypeToString() + " statement");

		if (token.m_Type == Token::While)
			return (Node*) new WhileStatement(condition, (BlockStatement*)body);

		assert(token.m_Type == Token::If);

		// If no else
		Token elseToken = parser.PeekToken(0);
		if (!parser.MatchToken(Token::Else))
			return (Node*) new IfStatement(condition, (BlockStatement*)body, nullptr);

		// Try to parse for else
		Node* elseArm = parser.Parse();
		if (parser.HasError()) return nullptr;

		if (!IsValidElseBody(elseArm))
			return parser.MakeErrorButPretty("Unexpected token after 'else' in if statement", elseToken);

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
				return parser.MakeErrorButPretty("No closing curly bracket for block statement", token);

			Node* line = parser.Parse();
			if (parser.HasError()) return nullptr;

			if (!line)
				return parser.MakeErrorButPretty("Could not parse block statement");

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
			return parser.MakeErrorButPretty("Found no closing parentheses for conditional statement");

		Token afterParantheses = parser.PeekToken(0);
		Node* body = parser.Parse();
		if (parser.HasError()) return nullptr;

		if (!body || body->m_Type != NodeType::BlockStatement)
			return parser.MakeErrorButPretty("Expected left curly bracket after " + token.TypeToString() + " statement", afterParantheses);

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

	Node* ParseVariableDeclaration(Parser& parser, Token token, Type* type, Identifier* name, bool consumeEndToken = true, Token::Types endToken = Token::Semicolon)
	{
		// int a;
		if (consumeEndToken)
		{
			if (parser.MatchToken(endToken))
				return new VariableDeclaration(type, name, nullptr);
		}
		else
		{
			if (parser.MatchTokenNoConsume(endToken))
				return new VariableDeclaration(type, name, nullptr);
		}

		// int a = ...;
		parser.ConsumeToken(Token::SetEquals);
		if (parser.HasError()) return nullptr;

		Node* assignedValue = parser.ParseExpression();
		if (parser.HasError()) return nullptr;
		
		if (!assignedValue)
			return parser.MakeErrorButPretty("Expected expression on right hand side of assignment, but got " + 
				parser.PeekToken(0).ToFormattedValueString(), parser.PeekToken(0));

		if (parser.HasError()) return nullptr;

		if (consumeEndToken)
			parser.ConsumeToken(endToken);

		return new VariableDeclaration(type, name, assignedValue);
	}

	std::tuple<Type*, Identifier*> ParseTypenameAndName(Parser& parser, Token token)
	{
		std::string variableType = token.m_Value;

		token = parser.PeekToken(0);

		if (parser.TokenIsTypename(token))
		{
			parser.MakeErrorButPretty("Cannot have two types next to each other in statement", token);
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

				Node* variableDeclaration = ParseVariableDeclaration(parser, token, type, name, false /* dont consume comma */, endToken);

				parameters.push_back(variableDeclaration);
			} while (parser.MatchToken(Token::Comma));

			parser.ConsumeToken(Token::RightParentheses);
		}

		return parameters;
	}

	Node* ParseFunctionDefinition(Parser& parser, Token token, Type* returnType, Identifier* name)
	{
		std::vector<Node*> parameters = ParseFunctionPrototypeParameters(parser, token);

		// Expect either a semicolon (function prototype) or a body
		Token nextToken = parser.PeekToken(0);
		if (parser.MatchToken(Token::Semicolon))
			return new FunctionDefinitionStatement(returnType, name, parameters, nullptr);

		// Otherwise parse body
		Node* body = parser.Parse();
		if (parser.HasError()) return nullptr;
		if (!body || body->m_Type != NodeType::BlockStatement)
			return parser.MakeErrorButPretty("Expected block statement for function definition", nextToken);

		return new FunctionDefinitionStatement(returnType, name, parameters, (BlockStatement*)body);
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
		return ParseVariableDeclaration(parser, token, type, name);
	}

	Node* ClosureParselet::Parse(Parser& parser, Token token)
	{
		Node* body = parser.Parse();
		if (parser.HasError()) return nullptr;
		if (!body || body->m_Type != NodeType::BlockStatement)
			return parser.MakeErrorButPretty("Expected block statement for closure", token);

		return new ClosureExpression((BlockStatement*)body);
	}

	Node* LoopParselet::Parse(Parser& parser, Token token)
	{
		Node* body = parser.Parse();
		if (!body || body->m_Type != NodeType::BlockStatement)
			return parser.MakeErrorButPretty("Expected block statement after 'loop'", token);
		if (parser.HasError()) return nullptr;

		return new LoopStatement((BlockStatement*)body);
	}

	Node* BreakParselet::Parse(Parser& parser, Token token)
	{
		Node* breakValue = parser.ParseExpression();
		parser.ConsumeToken(Token::Semicolon);
		if (parser.HasError()) return nullptr;

		return (Node*) new BreakStatement(breakValue);
	}

	Node* ClassDefinitionParselet::Parse(Parser& parser, Token token)
	{
		auto nameOpt = parser.ConsumeToken(Token::Identifier);
		if (!nameOpt.has_value()) return nullptr;

		std::string className = nameOpt.value().m_Value;

		if (parser.m_TypeTable.HasType(className))
			return parser.MakeErrorButPretty("Class '" + className + "' has already been declared elsewhere");

		parser.m_TypeTable.Add(className, TypeTableType::Class);

		Token nextToken = parser.ConsumeToken();
		if (parser.HasError()) return nullptr;

		Identifier* name = new Identifier(className);

		// Parse body of class definition
		ClassDeclarationStatement* classDeclaration = new ClassDeclarationStatement(name);

		while (true)
		{
			token = parser.PeekToken(0);
			if (parser.MatchToken(Token::RightCurlyBracket))
				break;

			if (parser.MatchToken(Token::EndOfFile))
				return parser.MakeErrorButPretty("No closing curly bracket for class definition", token);

			Node* line = parser.Parse();
			if (parser.HasError()) return nullptr;
			if (!line)
				return parser.MakeErrorButPretty("Could not parse class definition");

			// Add the node to the right spot
			if (line->m_Type == NodeType::VariableDeclaration)
				classDeclaration->m_MemberDeclarations.push_back((VariableDeclaration*)line);
			else if (line->m_Type == NodeType::FunctionDefinition)
				classDeclaration->m_MethodDeclarations.push_back((FunctionDefinitionStatement*)line);
			else if (line->m_Type == NodeType::ClassDeclaration)
				classDeclaration->m_NestedClassDeclarations.push_back((ClassDeclarationStatement*)line);
			else if (line->m_Type != NodeType::EmptyStatement)
				return parser.MakeErrorButPretty("Unsupported statement in class declaration '" + line->TypeToString() + "'", token);
		}

		//Node* definition = ParseClassDefinitionBody(parser, nextToken);
		if (parser.HasError()) return nullptr;

		// A semicolon after class declaration is legal, but not necessary
		//if (parser.MatchTokenNoConsume(Token::Semicolon))
			//parser.ConsumeToken(Token::Semicolon);

		return classDeclaration;
	}
}