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

namespace O::AST
{
	Node* ParseFunctionDefinition(Parser& parser, Token token, Type* returnType, Identifier* name);

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

	Node* ParseTupleExpression(Parser& parser)
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

		return new TupleExpression(arguments);
	}

	Node* ParenthesesGroupParselet::Parse(Parser& parser, Token token)
	{
		// Look ahead for a comma inside the parantheses
		int i = 0;
		bool isTuple = false;
		while (false)
		{
			Token peekToken = parser.PeekToken(i);

			if (peekToken.m_Type == Token::EndOfFile)
				return parser.MakeErrorButPretty("No closing parentheses found", token);

			if (peekToken.m_Type == Token::LeftParentheses)
				break;

			// If ')' is found before a comma, then it is not a tuple
			// But if it is the first token we peek, '() would it look like', then it is an empty tuple
			if (peekToken.m_Type == Token::RightParentheses)
			{
				// Tuple!
				if (i == 0)
					isTuple = true;

				//if (isTuple)
				{
					// If it has the format '(...) =>' then it is a lambda
					if (parser.MatchTokenNoConsume(i + 1, Token::RightArrow))
						return ParseFunctionDefinition(parser, token, nullptr, nullptr);

					if (parser.MatchTokenNoConsume(i + 1, Token::LeftCurlyBracket))
						return parser.MakeErrorButPretty("Expected '=>' after lamda parameters, block scopes are not supported in lambda");

					// Otherwise a normal tuple
					return ParseTupleExpression(parser);
				}

				// No tuple :(
				break;
			}

			// It is a tuple!
			if (peekToken.m_Type == Token::Comma)
				isTuple = true;

			i++;
		}

		// Parsing a normal group of parentheses
		Node* expression = parser.ParseExpression();

		auto result = parser.ConsumeToken(Token::RightParentheses);
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
		TupleExpression* arguments = (TupleExpression*)ParseTupleExpression(parser);

		if (parser.HasError()) return nullptr;

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

	Node* ParseFunctionDefinition(Parser& parser, Token token, Type* returnType, Identifier* name)
	{
		TupleExpression* parameters = ParseTuple(parser, token);

		// Validate the parameters
		for (auto& parameter : parameters->m_Elements)
		{
			// todo: use the  token that has the error
			if (parameter->m_Type != NodeType::VariableDeclaration)
				return parser.MakeErrorButPretty("Invalid function prototype, expected variable declaration"); 
		}

		// Expect either a semicolon (function prototype) or a body
		Token nextToken = parser.PeekToken(0);
		if (parser.MatchToken(Token::Semicolon))
			return new FunctionDefinitionStatement(returnType, name, parameters, nullptr);

		// Check if an expression function (int a() => 5);
		nextToken = parser.PeekToken(0);
		if (parser.MatchToken(Token::RightArrow))
		{
			if (parser.PeekToken().m_Type == Token::LeftCurlyBracket)
				return parser.MakeErrorButPretty("Expected expression after right arrow", nextToken);

			Node* expression = parser.ParseExpression();
			if (parser.HasError()) return nullptr;

			if (!expression)
				return parser.MakeErrorButPretty("Expected expression after right arrow", nextToken);

			//if (!parser.MatchTokenNoConsume(Token::Semicolon))
				//return parser.MakeErrorButPretty("Expected ';' after lamba expression");
			//if (parser.HasError()) return nullptr;

			return new FunctionDefinitionStatement(returnType, name, parameters, expression);
		}

		// Otherwise parse body
		Node* body = parser.Parse();
		if (parser.HasError()) return nullptr;
		if (!body || body->m_Type != NodeType::BlockStatement)
			return parser.MakeErrorButPretty("Expected block statement or '=>' after function definition", nextToken);

		return new FunctionDefinitionStatement(returnType, name, parameters, body);
	}

	Node* TypenameStatementParselet::Parse(Parser& parser, Token token)
	{
		// The statement could now be either a variable declaration, or a function declaration
		// int a; int a = ...; int a(...); int a (...) {...}

		auto [type, name] = parser.ParseTypeAndName(token);
		if (parser.HasError()) return nullptr;

		// If function
		if (parser.MatchToken(Token::LeftParentheses))
			return ParseFunctionDefinition(parser, token, type, name);

		// Otherwise it's a variable declaration (int a; or int a = ...;)
		return parser.ParseVariableDeclaration(token, type, name);
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

		return classDeclaration;
	}

	Node* LambdaParselet::Parse(Parser& parser, Node* left, Token token)
	{
		//Node* body = parser.ParseExpression();

		auto opOp = parser.m_DefinedOperators.GetBinary(token.m_Type);
		assert(opOp.has_value());
		auto& op = opOp.value();

		//Token nextToken = parser.PeekToken(0);
		//if (nextToken.IsOperator())
			//return parser.MakeError("Cannot have two binary operators next to each other");

		// Detemine if to parse the body as a statement or an expression
		Node* body = nullptr;
		if (parser.PeekToken().m_Type == Token::LeftCurlyBracket)
			body = parser.Parse();
		else
			body = parser.ParseExpression(op.GetParsePrecedence());

		if (parser.HasError())
			return nullptr;

		if (!body)
			return parser.MakeErrorButPretty("Expected body for lambda", token);

		return new LambdaExpression(nullptr, (TupleExpression*)left, body);
	}
}