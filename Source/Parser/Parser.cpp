#include "Parser.h"

#include <iostream>

#include "../Utils.hpp"

#define RETURN_IF_ERROR() if (HasError()) return nullptr;

namespace Ö::AST
{
	Parser::Parser()
	{
		using namespace Operators;

		// Using the precedences from C++
		// https://en.cppreference.com/w/cpp/language/operator_precedence

		// Precedence: 3
		m_DefinedOperators.AddOperator(UnaryPlus, "+", Unary, Token::Add, 3, Right);
		m_DefinedOperators.AddOperator(UnaryMinus, "-", Unary, Token::Subtract, 3, Right);
		//m_DefinedOperators.AddOperator(LogicalNot, Unary, Token::Not, 3, Left);
		//m_DefinedOperators.AddOperator(BitwiseNot, Unary, Token::Not, 3, Left);
		//m_DefinedOperators.AddOperator(LogicalNot, Unary, Token::Not, 3, Left);

		// Precedence: 5
		m_DefinedOperators.AddOperator(Multiplication, "*", Binary, Token::Multiply, 5, Left);
		m_DefinedOperators.AddOperator(Division, "/", Binary, Token::Divide, 5, Left);

		// Precedence: 6
		m_DefinedOperators.AddOperator(Addition, "+", Binary, Token::Add, 6, Left);
		m_DefinedOperators.AddOperator(Subtraction, "-", Binary, Token::Subtract, 6, Left);


	}

	Node Parser::CreateRootNode()
	{
		Node node(nullptr);
		node.m_Type = NodeType::Root;
		
		return node;
	}

	Node* Parser::CreateAST(Tokens& tokens, Node* parent)
	{
		Node* node = nullptr;

		// Try to parse scopes and the main body of a program
		node = ParseScope(tokens, parent);
		RETURN_IF_ERROR();

		if (node) 
			return node;

		// class

		// statements

		// assignment

		// Binary expressions
		node = ParseBinaryExpression(tokens, parent);
		RETURN_IF_ERROR();

		if (node)
			return node;

		// Parentheses
		node = ParseParentheses(tokens, parent);
		RETURN_IF_ERROR();

		if (node)
			return node;

		// Single token nodes
		// If there are more tokens than 1, then the code has invalid syntax
		if (tokens.size() != 1)
			return MakeError("Token " + tokens[0].ToString() + " has tokens after it that it shouldn't have");

		Token& token = tokens[0];
		if (token.m_Type == Token::Variable)
		{
			abort();
			//node->type = ASTTypes::Variable;
			//node->stringValue = token.m_Value;
		}
		if (token.m_Type == Token::IntLiteral)
			return new IntLiteral(parent, std::stoi(token.m_Value));
		if (token.m_Type == Token::DoubleLiteral)
			return new DoubleLiteral(parent, StringToDouble(token.m_Value));
		if (token.m_Type == Token::FloatLiteral)
			abort();
		if (token.m_Type == Token::StringLiteral)
			return new StringLiteral(parent, token.m_Value);

		return nullptr;
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
			float arg = TemporaryEvaluator(expr->m_Argument);

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

	Node* Parser::ParseScope(Tokens& tokens, Node* parent)
	{
		if (tokens[0].m_Type == Token::LeftCurlyBracket || parent->m_Type == NodeType::Root)
		{
			LinesOfTokens lines;
			Scope* scope;

			// There are no first and last brackets to exlude if it's the root program, so iterate all tokens
			if (parent->m_Type == NodeType::Root)
			{
				lines = MakeScopeIntoLines(tokens, 0, tokens.size(), 0);
				scope = new Program(parent);
			}
			else
			{
				lines = MakeScopeIntoLines(tokens, 1, tokens.size() - 1, tokens[0].m_Depth);
				scope = new BlockStatement(parent);
			}

			// Finally, evaluate all lines
			for (auto& line : lines)
			{
				Node* statement = CreateAST(line, scope);
				RETURN_IF_ERROR();

				scope->m_Lines.push_back(statement);
			}

			return scope;
		}

		return nullptr;
	}

	Node* Parser::ParseParentheses(Tokens& tokens, Node* parent)
	{
		if (tokens[0].m_Type != Token::LeftParentheses)
			return nullptr;

		// Slice until next parentheses
		auto end = FindMatchingEndBracket(tokens, tokens[0]);
		if (!end.has_value())
			return MakeError("Found no matching right parenthesis");

		Tokens contents = SliceVector(tokens, 1, end.value());
		return CreateAST(contents, parent);
	}




	Node* Parser::ParseBinaryExpression(Tokens& tokens, Node* parent)
	{
		using namespace Operators;

		auto IsTokenAnOperator = [&](Token token) {
			for (auto& op : m_DefinedOperators.GetOperators())
			{
				if (token.m_Type == op.m_TokenType) return true;
			}
			return false;
		};

		auto IsBinaryOpUnary = [&](Operator op, Tokens& left, Tokens& right) {
			assert(op.m_Type == Operators::Binary);

			for (auto& unaryOp : m_DefinedOperators.GetOperators())
			{
				if (op.m_Name == unaryOp.m_Name || unaryOp.m_Type == Operators::Binary)
					continue;

				if (unaryOp.m_TokenType == op.m_TokenType)
				{
					if (unaryOp.m_Associaticity == Associativity::Right)
					{
						if (left.empty())
							return true;

						if (IsTokenAnOperator(left.back()))
							return true;
					}
					else if (unaryOp.m_Associaticity == Associativity::Left)
					{
						if (right.empty())
							return true;

						if (IsTokenAnOperator(left.front()))
							return true;
					}

					return false;
				}
			}

			return false;
		};

		auto IsParsingCorrectUnary = [&](Operator op, Tokens& left, Tokens& right) {
			assert(op.m_Type == Operators::Unary);

			if (op.m_Associaticity == Associativity::Right)
			{
				// +a
				if (left.empty())
					return true;

				// +-a (op: -)
				if (IsTokenAnOperator(left.back()))
					return false;
			}
			else if (op.m_Associaticity == Associativity::Left)
			{
				if (right.empty())
					return true;

				if (IsTokenAnOperator(left.front()))
					return false;
			}

			return true;
		};

		// 1 - -2 * 5
		// 1 - 2
		// a+ + 5 => a+

		auto ParseOperator = [&](Operator& op, int positionOfOperator, bool& continueParsing) -> Node* {
			Tokens leftSide = SliceVector(tokens, 0, positionOfOperator);
			Tokens rightSide = SliceVector(tokens, positionOfOperator + 1);

			if (op.m_Type == Operators::Binary)
			{
				if (IsBinaryOpUnary(op, leftSide, rightSide))
				{
					continueParsing = true;
					return nullptr;
				}

				if (leftSide.empty())
				{
					

					MakeError("Expected something to the left of " + op.ToString());
					return nullptr;
				}

				BinaryExpression* binaryExpressionNode = new BinaryExpression(parent, op);
				binaryExpressionNode->m_Lhs = CreateAST(leftSide, binaryExpressionNode);
				RETURN_IF_ERROR();

				if (IsBinaryOpUnary(op, leftSide, rightSide))
				{
					continueParsing = true;
					return nullptr;
				}

				if (rightSide.empty())
				{
					MakeError("Expected something to the right of " + op.ToString());
					return nullptr;
				}

				binaryExpressionNode->m_Rhs = CreateAST(rightSide, binaryExpressionNode);
				RETURN_IF_ERROR();

				return binaryExpressionNode;
			}
			else if (op.m_Type == Operators::Unary)
			{
				if (!IsParsingCorrectUnary(op, leftSide, rightSide))
				{
					continueParsing = true;
					return nullptr;
				}

				UnaryExpression* unaryExpressionNode = new UnaryExpression(parent, op);

				// ex: a++. operands are to the left
				if (op.m_Associaticity == Operators::Associativity::Left)
				{
					if (leftSide.empty())
						return MakeError("Expected something to the left of " + op.ToString());
					if (!rightSide.empty())
						return MakeError("Didn't expect something to the right of " + op.ToString());

					unaryExpressionNode->m_Argument = CreateAST(leftSide, unaryExpressionNode);
				} else if(op.m_Associaticity == Operators::Associativity::Right)
				{
					if (rightSide.empty())
						return MakeError("Expected something to the right of " + op.ToString());
					if (!leftSide.empty())
						return MakeError("Didn't expect something to the left of " + op.ToString());

					unaryExpressionNode->m_Argument = CreateAST(rightSide, unaryExpressionNode);
				}
				
				RETURN_IF_ERROR();

				return unaryExpressionNode;
			}

			abort();
			return nullptr;
		};

		// Iterate all operators. They are already sorted by lowest precedence first
		// TODO: Associativity
		for (int opI = 0; opI < m_DefinedOperators.GetOperators().size(); opI++)
		{
			auto& op = m_DefinedOperators.GetOperators()[opI];

			// For some reason, left associative operators like add and subtract needs to iterate from the right
			// I would expect that they would start from the left instead.
			// But my guess is its because the same reason operators that goes first needs to be parsed last.
			// TODO: Prettify code somehow
			if (op.m_Associaticity == Associativity::Left)
			{
				for (int i = tokens.size() - 1; i >= 0; i--)
				{
					if (tokens[i].m_Type == op.m_TokenType)
					{
						if (IsInsideBrackets(tokens, i))
							continue;

						// 2 - 3
						// 2 + -3 * 6

						// error cases
						// binop: (not right associative unary op) op (not left associative unary op)

						// Check so the next token isn't an operator
						/*for (auto& op2 : m_DefinedOperators.GetOperators())
						{
							if (op.m_Type == Operators::Binary && !IsBinaryOpUnary(op))
							{
								if (!ElementExists(tokens, i - 1))
									return MakeError("err");
								if (!ElementExists(tokens, i + 1))
									return MakeError("err");

								if (tokens[i - 1].m_Type == op2.m_TokenType)
								{
									if (op2.m_Type == Operators::Binary ||
										(op2.m_Type == Operators::Unary && op2.m_Associaticity == Operators::Left))
										return MakeError("Cannot have " + op2.ToString() + " next to " + op.ToString());
								}

								if (tokens[i + 1].m_Type == op2.m_TokenType)
								{
									if (op2.m_Type == Operators::Binary ||
										(op2.m_Type == Operators::Unary && op2.m_Associaticity == Operators::Right))
										return MakeError("Cannot have " + op2.ToString() + " next to " + op.ToString());
								}
							}
						}*/

						bool continueParsing = false;
						Node* node = ParseOperator(op, i, continueParsing);

						if (continueParsing)
							continue;

						return node;
					}
				}
			} else if (op.m_Associaticity == Associativity::Right)
			{
				for (int i = 0; i < tokens.size(); i++)
				{
					if (tokens[i].m_Type == op.m_TokenType)
					{
						if (IsInsideBrackets(tokens, i))
							continue;

						bool continueParsing = false;
						Node* node = ParseOperator(op, i, continueParsing);

						if (continueParsing)
							continue;

						return node;
					}
				}
			}
		}

		return nullptr;
	}

	// TODO: Understand my code :)
	LinesOfTokens Parser::MakeScopeIntoLines(Tokens tokens, int start, int end, int startingDepth)
	{
		LinesOfTokens lines;
		Tokens currentLine;

		int scopeDepth = startingDepth;// tokens[0].m_Depth;
		bool isInsideParentheses = false;
		bool isInsideDeeperScope = false;

		bool isInsideStatement = false;

		int currentCurlyBracketDepth = 0;
		int curlyBracketDepth = 0;
		int statementDepth = 0;
		bool insideStatementArgs = false;

		bool dontReduceDepth = tokens[0].m_Depth == 0;

		auto ReduceTokenDepth = [dontReduceDepth](Token& token) {
			//if (!dontReduceDepth)
				//token.m_Depth--;
			};

		tokens = SliceVector(tokens, start, end);

		for (int i = 0; i < tokens.size(); i++)
		{
			Token token = tokens[i];

			isInsideDeeperScope = token.m_Depth > scopeDepth;
			bool isInsideStatement = statementDepth > 0;

			if (token.IsStatementKeyword())
			{
				statementDepth++;
				insideStatementArgs = true;
				ReduceTokenDepth(token);
				currentLine.push_back(token);
			}
			else if (token.m_Type == Token::LeftCurlyBracket)
			{
				ReduceTokenDepth(token);

				currentLine.push_back(token);

				curlyBracketDepth++;
			}
			else if (token.m_Type == Token::RightCurlyBracket)
			{
				ReduceTokenDepth(token);

				currentLine.push_back(token);

				curlyBracketDepth--;
				isInsideDeeperScope = token.m_Depth > scopeDepth;

				// If the curly bracket belongs to this scope
				if (!isInsideDeeperScope)
				{
					// Then search for elses that are right after the scope, for example an if statement
					/*if (i < tokens.size() - 1)
					{
						if (tokens[i + 1].m_Type == Token::Else)
						{
							currentLine.emplace_back(Token::Else, "", token.m_Depth - 1);
							i++;
						}
					}*/

					if (i < tokens.size() - 1)
					{
						if (tokens[i + 1].m_Type == Token::Else)
						{

						}
						else {
							lines.push_back(currentLine);
							currentLine.clear();
						}
					}

					// Reduce the scope depth
					/*scopeDepth--;
					isInsideDeeperScope = false;
					curlyBracketDepth--;*/
				}
			}
			else if (token.m_Type == Token::Else)
			{
				ReduceTokenDepth(token);
				currentLine.push_back(token);
			}
			else if (token.m_Type == Token::LeftParentheses)
			{
				isInsideParentheses = true;

				if (curlyBracketDepth > 0 || insideStatementArgs)
					ReduceTokenDepth(token);

				currentLine.push_back(token);
			}
			else if (token.m_Type == Token::RightParentheses)
			{
				isInsideParentheses = true;

				if (curlyBracketDepth > 0 || insideStatementArgs)
					ReduceTokenDepth(token);

				if (statementDepth == token.m_Depth) insideStatementArgs = false;

				currentLine.push_back(token);
			}
			/*else if (token.m_Type == Token::Else)
			{
				currentLine.emplace_back(Token::Else, "", token.m_Depth - 1);
			}*/
			else
			{
				if (token.m_Type != Token::Semicolon)
				{
					if (isInsideDeeperScope) {
						if (curlyBracketDepth > 0 || insideStatementArgs)
							ReduceTokenDepth(token);
					}

					currentLine.push_back(token);
				}
				else
				{
					if (!isInsideDeeperScope)
					{
						if (!currentLine.empty())
						{
							lines.push_back(currentLine);
							currentLine.clear();
						}
					}
					else
					{
						ReduceTokenDepth(token);
						currentLine.push_back(token);
					}
				}
			}
		}
		if (!currentLine.empty()) lines.push_back(currentLine);

		return lines;
	}


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
};