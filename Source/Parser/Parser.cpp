#include "Parser.h"

#include <iostream>

#include "../Utils.hpp"

#define RETURN_IF_ERROR() if (HasError()) return nullptr;

namespace Ö::AST
{
	Parser::Parser()
	{
		using namespace Operators;

		m_DefinedOperators.AddOperator(Addition, Binary, Token::Add, 10, Left);
		m_DefinedOperators.AddOperator(Subtraction, Binary, Token::Subtract , 10, Left);
		m_DefinedOperators.AddOperator(Multiplication, Binary, Token::Multiply, 20, Left);
		m_DefinedOperators.AddOperator(Division, Binary, Token::Divide, 20, Left);
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

	void Parser::PrintASTTree(Node* node, int depth)
	{
		if (!node)
			return;

		std::string padding = "";
		for (int i = 0; i < depth; i++)
			padding += "    ";

		std::cout << padding << node->TypeToString() << ": ";

		switch (node->m_Type)
		{
		case NodeType::Empty:
			break;
		case NodeType::Root:
			break;
		case NodeType::Program:
		case NodeType::BlockStatement:
		{
			Scope* scope = (Scope*)node;
			std::cout << "\n";

			std::string acc;
			for (int i = 0; i < scope->m_Lines.size(); i++)
			{
				PrintASTTree(scope->m_Lines[i], depth + 1);
			}
		}
			break;
		case NodeType::Identifier:
			std::cout << padding << node->ToString() << "\n";
			break;
		case NodeType::VariableDeclaration:
			break;
		case NodeType::AssignmentExpression:
			break;
		case NodeType::BinaryExpression:
		{
			BinaryExpression* expression = (BinaryExpression*)node;

			std::cout << expression->ToString() << "\n";

			PrintASTTree(expression->m_Left, depth + 1);
			//std::cout << padding << expression->ToString() << "\n";
			PrintASTTree(expression->m_Right, depth + 1);

			break;
		}		
		case NodeType::IntLiteral:
		case NodeType::FloatLiteral:
		case NodeType::DoubleLiteral:
		case NodeType::StringLiteral:
			std::cout << node->ToString() << "\n";
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




	Node* Parser::ParseBinaryExpression(Tokens& tokens, Node* parent)
	{
		using namespace Operators;

		auto ParseOperator = [&](Operator& op, int positionOfOperator) -> Node* {
			BinaryExpression* binaryExpressionNode = new BinaryExpression(parent);
			binaryExpressionNode->m_Operator = op;

			Tokens leftSide = SliceVector(tokens, 0, positionOfOperator);

			//ReduceDepthOfBrackets(leftSide, Token::LeftParentheses);

			if (leftSide.size() == 0)
			{
				// TODO: Handle unary ops
				MakeError("Expected something to the left of " + op.ToString());
				return nullptr;
			}

			binaryExpressionNode->m_Left = CreateAST(leftSide, binaryExpressionNode);
			RETURN_IF_ERROR();

			Tokens rightSide = SliceVector(tokens, positionOfOperator + 1);

			if (rightSide.size() == 0)
			{
				MakeError("Expected something to the right of " + op.ToString());
				return nullptr;
			}

			binaryExpressionNode->m_Right = CreateAST(rightSide, binaryExpressionNode);
			RETURN_IF_ERROR();

			return binaryExpressionNode;
		};

		// Iterate all operators. They are already sorted by lowest precedence first
		// TODO: Associativity
		for (auto& op : m_DefinedOperators.GetOperators())
		{
			for (int i = 0; i < tokens.size(); i++)
			{
				if (tokens[i].m_Type == op.m_TokenType)
				{
					if (IsInsideBrackets(tokens, i))
						continue;

					return ParseOperator(op, i);
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
			if (ElementExists(tokens, i - 1))
			{
				if (tokens[i - 1].m_Type == Token::LeftParentheses ||
					tokens[i - 1].m_Type == Token::LeftSquareBracket) {
					if (tokens[i].m_Depth == tokens[start].m_Depth)
						return true;
				}

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