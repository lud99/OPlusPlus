#include "Parser.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <fstream>

#include "Memory.h"
#include "Lexer.h"
#include "Utils.hpp"

std::string ASTNode::ToString(bool includeData)
{
	std::string names[] = {
		"Empty",
		"VariableDeclaration",
		"VariableType",
		"Assign",
		"PropertyAssign",
		"CompareEquals",
		"CompareNotEquals",
		"CompareLessThan",
		"CompareGreaterThan",
		"CompareLessThanEqual",
		"CompareGreaterThanEqual",
		"And",
		"Or",
		"Not",
		"Null",
		"IntLiteral",
		"StringLiteral",
		"Bool",
		"ArrayType",
		"FunctionType",
		"ObjectType",
		"Variable",
		"PropertyAccess",
		"ListInitializer",
		"MathExpression",
		"Add",
		"Subtract",
		"Multiply",
		"Divide",
		"Xor",
		"ToThePower",
		"Modulus",
		"PlusEquals",
		"MinusEquals",
		"PostIncrement",
		"PreIncrement",
		"PostDecrement",
		"PreDecrement",
		"ProgramBody",
		"ModuleBody",
		"Line",
		"FunctionCall",
		"Return",
		"IfStatement",
		"Else",
		"WhileStatement",
		"ForStatement",
		"FunctionDefinition",
		"AnonymousFunction",
		"Break",
		"Continue",
		"Import",
		"Export",
		"Scope",
		"Modifier"
	};

	if (type == ASTTypes::Empty) return "(empty)";

	std::string typeStr = names[(int)type];

	if (includeData)
	{
		std::string val = "(\"" + stringValue + "\", " + std::to_string(numberValue) + ")";

		return typeStr + " " + val;
	}

	return typeStr;
}

bool Parser::MakeError(const std::string& message)
{
	m_Error = message;
	return false;
}
void Parser::MakeErrorVoid(const std::string& message)
{
	m_Error = message;
}

bool Parser::HasError()
{
	return m_Error != "";
}

void Parser::PrintASTTree(ASTNode* node, int depth)
{
	if (node != nullptr)
	{
		std::string padding = "";
		for (int i = 0; i < depth; i++)
			padding += "    ";

		std::cout << padding << node->ToString() << "\n";

		PrintASTTree(node->left, depth + 1);
		PrintASTTree(node->right, depth + 1);
		for (int i = 0; i < node->arguments.size(); i++)
		{
			PrintASTTree(node->arguments[i], depth + 1);
		}
	}
}

bool IsInsideBrackets(std::vector<Token> tokens, int start)
{
	for (int i = start; i >= 0; i--) // Walk from here backwards
	{
		if (ElementExists(tokens, i - 1))
		{
			if (tokens[i - 1].m_Type == Token::LeftParentheses || 
				tokens[i - 1].m_Type == Token::LeftSquareBracket)
				return true;
		}
		if (tokens[i].m_Type == Token::RightParentheses ||
			tokens[i].m_Type == Token::RightSquareBracket)
		{
			assert(tokens[i].m_Depth != -1);

			return tokens[i].m_Depth != 1;
		}
	}
	
	return false;
}

// Find the matching bracket (, {, [ with the same depth
int FindMatchingEndBracket(std::vector<Token>& tokens, Token& startToken)
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

	return -1;
}

void ReduceDepthOfBrackets(std::vector<Token>& tokens, Token::Types type)
{
	Token::Types typeOfEnd = Token::Empty;
	if (type == Token::LeftParentheses)
		typeOfEnd = Token::RightParentheses;
	else if (type == Token::LeftCurlyBracket)
		typeOfEnd = Token::RightCurlyBracket;
	else if (type == Token::LeftSquareBracket)
		typeOfEnd = Token::RightSquareBracket;

	for (int i = 0; i < tokens.size(); i++)
	{
		assert(tokens[i].m_Depth != 0);

		if (tokens[i].m_Type == type || tokens[i].m_Type == typeOfEnd)
			tokens[i].m_Depth--;
	}
}


std::vector<std::vector<Token>> DepthSplit(std::vector<Token> tokens, Token::Types delimiter, int depth)
{
	std::vector<std::vector<Token>> splitted;
	std::vector<Token> currentEntry;

	for (int i = 0; i < tokens.size(); i++)
	{
		if (tokens[i].m_Type == delimiter && tokens[i].m_Depth == depth)
		{
			splitted.push_back(currentEntry);
			currentEntry.clear();
		}
		else
		{
			currentEntry.push_back(tokens[i]);
		}
	}

	if (!currentEntry.empty()) splitted.push_back(currentEntry);

	return splitted;
}


void ReduceDepthOfTokens(std::vector<Token>& tokens, int delta = -1)
{
	for (int i = 0; i < tokens.size(); i++)
	{
		assert(tokens[i].m_Depth != 0);
		tokens[i].m_Depth += delta;
	}
}

bool IsTokenValidPartOfExpression(Token token)
{
	//if (token.IsStatementKeyword())
		//return false;
	if (token.IsVariableType())
		return false;
	//if (token.m_Type == Token::FunctionType)
		//return true;

	return true;
}

//bool IsValidFunctionDeclaration(std::vector<Token>& tokens)
//{
//	if (tokens[0].m_Type == Token::FunctionType)
//	{
//		if (tokens[1].m_Type == Token::LeftParentheses)
//		{
//			if (tokens[2].m_Type == Token::RightParentheses)
//			{
//				if (tokens[3].m_Type == Token::LeftCurlyBracket)
//				{
//					return true;
//				}
//			}
//		}
//		else if (tokens[1].m_Type == Token::FunctionCall)
//		{
//			if (tokens[2].m_Type == Token::LeftParentheses)
//			{
//				if (tokens[3].m_Type == Token::RightParentheses)
//				{
//					if (tokens[4].m_Type == Token::LeftCurlyBracket)
//					{
//						return true;
//					}
//				}
//			}
//		}
//	}
//
//	return false;
//}

bool Parser::ParseMathExpression(Tokens& tokens, ASTNode* node)
{
	auto ParseMath = [&](int positionOfMathOperator) {
		node->left = new ASTNode;
		std::vector<Token> leftSide = SliceVector(tokens, 0, positionOfMathOperator);

		//ReduceDepthOfBrackets(leftSide, Token::LeftParentheses);

		if (leftSide.size() == 0)
		{
			// Assume that a '-' without a left means a multiplication by -1 
			// TODO: Might not always work because of order of operations
			if (node->type == ASTTypes::Subtract)
			{
				node->type = ASTTypes::Multiply;
				node->left->type = ASTTypes::IntLiteral;
				node->left->numberValue = -1.0f;
			}
			else
			{
				return MakeError("Expected something to the left of math operator");
			}
		}

		CreateAST(leftSide, node->left, node);
		if (HasError()) return false;

		node->right = new ASTNode;
		std::vector<Token> rightSide = SliceVector(tokens, positionOfMathOperator + 1);

		if (rightSide.size() == 0)
			return MakeError("Expected something to the right of math operator");

		CreateAST(rightSide, node->right, node);
		if (HasError()) return false;

		return true;
	};

	// Add
	for (int i = 0; i < tokens.size(); i++)
	{
		if (tokens[i].m_Type == Token::Add)
		{
			if (IsInsideBrackets(tokens, i))
				continue;
			node->type = ASTTypes::Add;
			return ParseMath(i);
		}
	}
	// Subtract
	for (int i = 0; i < tokens.size(); i++)
	{
		if (tokens[i].m_Type == Token::Subtract)
		{
			if (IsInsideBrackets(tokens, i))
				continue;
			node->type = ASTTypes::Subtract;
			return ParseMath(i);
		}
	}
	// Multiply
	for (int i = 0; i < tokens.size(); i++)
	{
		if (tokens[i].m_Type == Token::Multiply)
		{
			if (IsInsideBrackets(tokens, i))
				continue;
			node->type = ASTTypes::Multiply;
			return ParseMath(i);
		}
	}
	// Divide
	for (int i = 0; i < tokens.size(); i++)
	{
		if (tokens[i].m_Type == Token::Divide)
		{
			if (IsInsideBrackets(tokens, i))
				continue;
			node->type = ASTTypes::Divide;
			return ParseMath(i);
		}
	}

	return false;
}

void ReduceDepth(std::vector<Token>& tokens, Token::Types toFind)
{
	for (int i = 0; i < tokens.size(); i++)
	{
		if (tokens[i].m_Type == toFind)
			tokens[i].m_Depth--;
	}
}

std::vector<std::vector<Token>> MakeScopeIntoLines(std::vector<Token> tokens, int start, int end)
{
	std::vector<std::vector<Token>> lines;
	std::vector<Token> currentLine;

	int scopeDepth = 0;
	bool isInsideParentheses = false;
	bool isInsideDeeperScope = false;

	int currentCurlyBracketDepth = 0;
	int curlyBracketDepth = 0;

	tokens = SliceVector(tokens, start, end);

	for (int i = 0; i < tokens.size(); i++)
	{
		Token token = tokens[i];

		isInsideDeeperScope = token.m_Depth > scopeDepth;

		if (token.m_Type == Token::LeftCurlyBracket)
		{
			currentLine.emplace_back(Token::LeftCurlyBracket, "", token.m_Depth - 1);
			curlyBracketDepth++;
		}
		else if (token.m_Type == Token::RightCurlyBracket)
		{
			currentLine.emplace_back(Token::RightCurlyBracket, "", token.m_Depth - 1);
			curlyBracketDepth--;

			// If the curly bracket belongs to this scope
			if (token.m_Depth == 1)
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

				lines.push_back(currentLine);
				currentLine.clear();

				// Reduce the scope depth
				/*scopeDepth--;
				isInsideDeeperScope = false;*/
			}
		}
		else if (token.m_Type == Token::LeftParentheses)
		{
			isInsideParentheses = true;
			if (curlyBracketDepth > 0)
				token.m_Depth--;

			currentLine.push_back(token);
		}
		else if (token.m_Type == Token::RightParentheses)
		{
			isInsideParentheses = true;
			if (curlyBracketDepth > 0)
				token.m_Depth--;

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
					if (curlyBracketDepth > 0)
						token.m_Depth--;
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
					token.m_Depth--;
					currentLine.push_back(token);
				}
			}
		}
	}
	if (!currentLine.empty()) lines.push_back(currentLine);

	return lines;
}

// {type} {variable} = {expression} 
// {variable} = {expression} 
bool Parser::IsValidAssignmentExpression(Tokens tokens, int equalsSignPosition)
{
	if (!ElementExists(tokens, equalsSignPosition - 1) || tokens[equalsSignPosition - 1].m_Type != Token::Variable)
		return MakeError("Expected a variable on the left side of the equals sign, not a " + tokens[equalsSignPosition - 1].ToString());

	if (ElementExists(tokens, equalsSignPosition - 2))
	{
		// For this to be valid the token i - 2 has to be a type decleration
		if (!tokens[equalsSignPosition - 2].IsVariableType())// && tokens[equalsSignPosition - 2].m_Type != Token::PropertyAccess)
			return MakeError("Expected variable type to the left of variable in assignment, not a " + tokens[equalsSignPosition - 2].ToString());
	}

	if (!ElementExists(tokens, equalsSignPosition + 1))
		return MakeError("Expected an expression on the right side of the equals sign");

	//if (tokens[equalsSignPosition + 1].m_Type == Token::FunctionType)
		//return true;

	if (!IsTokenValidPartOfExpression(tokens[equalsSignPosition + 1]))
		return MakeError("Expected an expression on the right side of the equals sign, not a " + tokens[equalsSignPosition + 1].ToString());

	return true;
}

//// {variable}: {expression} 
//bool Parser::IsValidPropertyAssignmentExpression(std::vector<Token> tokens)
//{
//	if (!ElementExists(tokens, 1) || tokens[0].m_Type != Token::Variable)
//		return CreateError("Expected a variable on the left side of the equals sign, not a " + tokens[0].ToString());
//
//	if (!ElementExists(tokens, 2))
//		return CreateError("Expected an expression on the right side of the equals sign");
//
//
//	if (!IsTokenValidPartOfExpression(tokens[2]))
//		return CreateError("Expected an expression on the right side of the equals sign, not a " + tokens[2].ToString());
//
//	return true;
//}
//

// {variable} += {expression}
bool Parser::IsValidCompoundAssignmentExpression(Tokens tokens, int operatorPosition)
{
	Token opToken = tokens[operatorPosition];

	if (!ElementExists(tokens, operatorPosition - 1))
		return MakeError("Expected something before " + opToken.ToString());

	Token varToken = tokens[operatorPosition - 1];

	if (varToken.m_Type != Token::Variable)
		return MakeError("Expected a variable before " + opToken.ToString() + " but got a " + tokens[operatorPosition - 1].ToString());

	// If more things before variable
	if (ElementExists(tokens, operatorPosition - 2))
		return MakeError("Expected only a variable on the left side of " + opToken.ToString() + " but got" + tokens[operatorPosition - 2].ToString());

	if (!ElementExists(tokens, operatorPosition + 1))
		return MakeError("Expected an expression after " + opToken.ToString());

	return true;
}

// {something} && {something}
bool Parser::IsValidLogicalAndOrExpression(Tokens tokens, int position)
{
	if (!ElementExists(tokens, position - 1))
		return MakeError("Expected something to the left of " + tokens[position].ToString() + " operator");
	if (!ElementExists(tokens, position + 1))
		return MakeError("Expected something to the right of " + tokens[position].ToString() + " operator");

	return true;
}

bool Parser::IsValidComparisonExpression(Tokens tokens, int position)
{
	if (!ElementExists(tokens, position - 1))
		return MakeError("Expected comething to the left of " + tokens[position].ToString() + " operator");
	if (!ElementExists(tokens, position + 1))
		return MakeError("Expected comething to the right of " + tokens[position].ToString() + " operator");

	return true;
}

// {variable}++
bool Parser::IsValidPostIncDecExpression(Tokens tokens, int position)
{
	if (tokens[position].m_Type != Token::Variable)
		return MakeError("Expected variable to the left of Increment or decrement");

	if (!ElementExists(tokens, position + 1))
		return MakeError("Expected ++ or -- to the right of variable");

	if (tokens[position + 1].m_Type != Token::PostIncrement && tokens[position + 1].m_Type != Token::PostDecrement)
		return MakeError("Expected ++ or -- to the right of variable, not " + tokens[position + 1].ToString());

	if (tokens.size() > 2)
		return MakeError("Too many things in increment or decrement expression");

	return true;
}

// ++{variable}
bool Parser::IsValidPreIncDecExpression(Tokens tokens, int position)
{
	if (tokens[position].m_Type != Token::PreIncrement && tokens[position].m_Type != Token::PreDecrement)
		return MakeError("Expected ++ or -- to the left of variable, not " + tokens[position].ToString());

	if (!ElementExists(tokens, position + 1))
		return MakeError("Expected variable to the right of " + tokens[position].ToString());

	if (tokens[position + 1].m_Type != Token::Variable)
		return MakeError("Expected variable to the right of " + tokens[position].ToString());

	if (tokens.size() > 2)
		return MakeError("Too many things in increment or decrement expression");

	return true;
}

bool Parser::IsValidFunctionCallExpression(Tokens tokens)
{
	if (tokens[0].m_Type != Token::FunctionName)
		return MakeError("Not a function call");
	if (!ElementExists(tokens, 1) || tokens[1].m_Type != Token::LeftParentheses)
		return MakeError("Expected a left parentheses after function call, but got " + tokens[1].ToString());

	int endParanthesisPosition = FindMatchingEndBracket(tokens, tokens[1]);
	if (endParanthesisPosition == -1)
		return MakeError("Found no closing parenthesis for function call");

	std::vector<Token> argContent = SliceVector(tokens, 2, endParanthesisPosition);
	if (argContent.front().m_Type == Token::Comma)
		return MakeError("Expected an argument before the first comma in the function call");
	if (argContent.back().m_Type == Token::Comma)
		return MakeError("Expected an argument after the last comma in the function call");

	return true;
}

// {type} {variable}
bool Parser::IsValidVariableDeclarationExpression(Tokens tokens)
{
	if (!tokens[0].IsVariableType())
		return false;

	if (!ElementExists(tokens, 1) || tokens[1].m_Type != Token::Variable)
		return MakeError("Expected a variable after variable type");
	return true;
}

void Parser::CreateAST(std::vector<Token>& tokens, ASTNode* node, ASTNode* parent)
{
	node->parent = parent;

	if (HasError()) return;

	if (tokens.empty()) return;

	// Check for scopes
	if (tokens[0].m_Type == Token::LeftCurlyBracket || parent->type == ASTTypes::ProgramBody)
	{
		node->type = ASTTypes::Scope;

		std::vector<std::vector<Token>> lines;

		// There are no first and last brackets to exlude if it's the root program, so iterate all tokens
		if (parent->type == ASTTypes::ProgramBody)
			lines = MakeScopeIntoLines(tokens, 0, tokens.size());
		else
			lines = MakeScopeIntoLines(tokens, 1, tokens.size() - 1);

		// Evaluate each of the lines, and make the result a child of a line node
		ASTNode* lineNode = new ASTNode;    

		// Try to merge lines that end with 'else'
		std::vector<std::vector<Token>> mergedLines;
		std::vector<Token> concatResult;

		//bool foundElse = false;
		//for (int i = 0; i < lines.size(); i++)
		//{
		//	if (!lines[i].empty() && lines[i].back().m_Type == Token::Else)
		//	{
		//		// Merge the two lines
		//		if (concatResult.empty())
		//			concatResult = lines[i];

		//		concatResult = ConcatVectors(concatResult, lines[i + 1]);
		//		foundElse = true;
		//	}
		//	else
		//	{
		//		if (!concatResult.empty())
		//			mergedLines.push_back(concatResult);

		//		if (!foundElse) 
		//			mergedLines.push_back(lines[i]);

		//		concatResult.clear();
		//		foundElse = false;
		//	}			
		//}
		//lines = mergedLines;

		// Finally, evaluate all lines
		for (int i = 0; i < lines.size(); i++)
		{
			ASTNode* line = new ASTNode;

			CreateAST(lines[i], line, node);
			if (HasError()) return;
		
			node->arguments.push_back(line);
		}

		return;
	}

	// Check for else
	//

	// if statements

	if (!ParseAssignment(tokens, node))
	{
		if (HasError())
			return;
	}
	else return;

	if (!ParseCompoundAssignment(tokens, node))
	{
		if (HasError())
			return;
	}
	else return;

	// parse else 

	// parse return 

	if (!ParseLogicalAndOr(tokens, node))
	{
		if (HasError())
			return;
	}
	else return;

	if (!ParseComparisonOperators(tokens, node))
	{
		if (HasError())
			return;
	}
	else return;

	if (!ParseMathExpression(tokens, node))
	{
		if (HasError())
			return;
	}
	else return;

	if (!ParseParentheses(tokens, node))
	{
		if (HasError())
			return;
	}

	if (!ParseIncrementDecrement(tokens, node))
	{
		if (HasError())
			return;
	}
	else return;

	if (!ParseFunctionCall(tokens, node))
	{
		if (HasError())
			return;
	}
	else return;

	if (!ParseVariableDeclaration(tokens, node))
	{
		if (HasError())
			return;
	}
	else return;

	// Single token nodes
	if (tokens.size() == 1)
	{
		Token& token = tokens[0];
		if (token.m_Type == Token::Variable)
		{
			node->type = ASTTypes::Variable;
			node->stringValue = token.m_Value;
		}
		if (token.m_Type == Token::IntLiteral)
		{
			node->type = ASTTypes::IntLiteral;
			node->numberValue = std::stoi(token.m_Value);
		}
		if (token.m_Type == Token::StringLiteral)
		{
			node->type = ASTTypes::StringLiteral;
			node->stringValue = token.m_Value;
		}

		return;
	}

	return;
}

bool Parser::ParseAssignment(Tokens& tokens, ASTNode* node)
{
	for (int i = 0; i < tokens.size(); i++)
	{
		if (tokens[i].m_Type == Token::SetEquals)
		{
			if (IsInsideBrackets(tokens, i))
				continue;

			if (!IsValidAssignmentExpression(tokens, i))
				return false;

			node->type = ASTTypes::Assign;

			std::vector<Token> lhs = SliceVector(tokens, 0, i);
			std::vector<Token> rhs = SliceVector(tokens, i + 1);

			node->left = new ASTNode;
			node->right = new ASTNode;

			CreateAST(lhs, node->left, node);
			CreateAST(rhs, node->right, node);

			return true;
		}
	}

	return false;
}

bool Parser::ParseLogicalAndOr(Tokens& tokens, ASTNode* node)
{
	for (int i = 0; i < tokens.size(); i++)
	{
		if (tokens[i].m_Type == Token::And || tokens[i].m_Type == Token::Or)
		{
			if (IsInsideBrackets(tokens, i))
				continue;

			if (!IsValidLogicalAndOrExpression(tokens, i))
				return false;

			if (tokens[i].m_Type == Token::And)
				node->type = ASTTypes::And;
			if (tokens[i].m_Type == Token::Or)
				node->type = ASTTypes::Or;

			node->left = new ASTNode;
			Tokens leftTokens = SliceVector(tokens, 0, i);
			CreateAST(leftTokens, node->left, node);
			if (HasError()) return false;

			node->right = new ASTNode;
			Tokens rightTokens = SliceVector(tokens, i + 1);
			CreateAST(rightTokens, node->right, node);
			if (HasError()) return false;

			return true;
		}
	}

	return false;
}

bool Parser::ParseComparisonOperators(Tokens& tokens, ASTNode* node)
{
	for (int i = 0; i < tokens.size(); i++)
	{
		if (tokens[i].IsComparisonOperator())
		{
			if (IsInsideBrackets(tokens, i))
				continue;

			if (!IsValidComparisonExpression(tokens, i))
				return false;

			if (tokens[i].m_Type == Token::CompareEquals)
				node->type = ASTTypes::CompareEquals;
			else if (tokens[i].m_Type == Token::NotEquals)
				node->type = ASTTypes::CompareNotEquals;
			else if (tokens[i].m_Type == Token::LessThan)
				node->type = ASTTypes::CompareLessThan;
			else if (tokens[i].m_Type == Token::GreaterThan)
				node->type = ASTTypes::CompareGreaterThan;
			else if (tokens[i].m_Type == Token::LessThanEqual)
				node->type = ASTTypes::CompareLessThanEqual;
			else if (tokens[i].m_Type == Token::GreaterThanEqual)
				node->type = ASTTypes::CompareGreaterThanEqual;

			node->left = new ASTNode;
			node->right = new ASTNode;

			std::vector<Token> lhs = SliceVector(tokens, 0, i);
			std::vector<Token> rhs = SliceVector(tokens, i + 1);

			CreateAST(lhs, node->left, node);
			CreateAST(rhs, node->right, node);

			return true;
		}
	}

	return false;
}

bool Parser::ParseCompoundAssignment(Tokens& tokens, ASTNode* node)
{
	// Plus/Minus Equals. left += right
	for (int i = 0; i < tokens.size(); i++)
	{
		if (tokens[i].m_Type == Token::PlusEquals || tokens[i].m_Type == Token::MinusEquals)
		{
			if (IsInsideBrackets(tokens, i))
				continue;

			if (!IsValidCompoundAssignmentExpression(tokens, i))
				return false;

			node->type = ASTTypes::Assign;

			node->left = new ASTNode;
			Tokens newTokens = SliceVector(tokens, 0, i);
			CreateAST(newTokens, node->left, node);
			if (HasError()) return false;

			node->right = new ASTNode;
			node->right->left = new ASTNode;

			if (tokens[i].m_Type == Token::PlusEquals)
				node->right->type = ASTTypes::Add;
			else if (tokens[i].m_Type == Token::MinusEquals)
				node->right->type = ASTTypes::Subtract;

			node->right->left->type = ASTTypes::Variable;
			node->right->left->stringValue = tokens[i - 1].m_Value;

			node->right->right = new ASTNode;
			newTokens = SliceVector(tokens, i + 1);

			if (newTokens.size() == 0)
				return MakeError("Expected something after " + tokens[i].ToString() + "sign");

			CreateAST(newTokens, node->right->right, node->right);
			if (HasError()) return false;

			return true;
		}
	}

	return false;
}

bool Parser::ParseVariableDeclaration(Tokens& tokens, ASTNode* node)
{
	if (!IsValidVariableDeclarationExpression(tokens))
		return false;

	node->type = ASTTypes::VariableDeclaration;
	node->left = new ASTNode;
	node->left->type = ASTTypes::VariableType;
	node->left->stringValue = tokens[0].m_Value;

	node->left = new ASTNode;
	node->left->type = ASTTypes::Variable;
	node->left->stringValue = tokens[1].m_Value;

	return true;
}

bool Parser::ParseParentheses(Tokens& tokens, ASTNode* node)
{
	// Paranthesis
	if (tokens[0].m_Type != Token::LeftParentheses)
		return false;
	
	// Slice until next parenthesis
	int end = FindMatchingEndBracket(tokens, tokens[0]);
	if (end == -1)
	{
		MakeError("Found no matching right parenthesis");
		return false;
	}

	if (end == 2) // Theres only on item in the brackets
	{
		std::vector<Token> newTokens = tokens;

		// Remove the brackets and act as normal
		newTokens.erase(newTokens.begin() + end);
		newTokens.erase(newTokens.begin() + 0);

		CreateAST(newTokens, node, node->parent);
		if (HasError()) return false;
	}
	else
	{
		/*if (end + 1 < tokens.size())
		{
			std::vector<Token> newTokens = SliceVector(tokens, end + 1);
			node->left = new ASTNode;
			error = CreateAST(newTokens, node->left, node);
			if (error != "") return error;
		}
		else*/
		{
			std::vector<Token> newTokens = SliceVector(tokens, 1, end);
			ReduceDepthOfTokens(newTokens);

			CreateAST(newTokens, node, node->parent);
		}

		if (HasError()) return false;
	}

	return true;
}

bool Parser::ParseFunctionCall(Tokens& tokens, ASTNode* node)
{
	if (tokens[0].m_Type != Token::FunctionName)
		return false;

	if (IsInsideBrackets(tokens, 0))
		return false;

	if (!IsValidFunctionCallExpression(tokens))
		return false;

	node->type = ASTTypes::FunctionCall;
	node->stringValue = tokens[0].m_Value; // Function Name

	int endParanthesisPosition = FindMatchingEndBracket(tokens, tokens[1]);
	std::vector<Token> argContent = SliceVector(tokens, 2, endParanthesisPosition);

	// No arguments (print());
	if (argContent.empty())
		return true;

	std::vector<Tokens> arguments = DepthSplit(argContent, Token::Comma, tokens[1].m_Depth);

	// Resolve the arguments
	for (int i = 0; i < arguments.size(); i++)
	{
		// If an argument has no tokens, then there was nothing after the comma
		if (arguments[i].empty())
			return MakeError("Expected an argument after the comma in the function call");

		ReduceDepthOfBrackets(arguments[i], Token::LeftParentheses);
		ReduceDepth(arguments[i], Token::Comma);

		ASTNode* argNode = new ASTNode;
		CreateAST(arguments[i], argNode, node);

		node->arguments.push_back(argNode);
	}

	return true;
}

bool Parser::ParseIncrementDecrement(Tokens& tokens, ASTNode* node)
{
	for (int i = 0; i < tokens.size(); i++)
	{
		if ((tokens[i].m_Type == Token::PostIncrement || tokens[i].m_Type == Token::PostDecrement))
		{
			// Check if these tokens are inside a function argument
			if (IsInsideBrackets(tokens, i))
				continue;

			if (!IsValidPostIncDecExpression(tokens, i))
				return false;

			if (tokens[i].m_Type == Token::PostIncrement)
				node->type = ASTTypes::PostIncrement;
			else if (tokens[i].m_Type == Token::PostDecrement)
				node->type = ASTTypes::PostDecrement;

			// The left side should be a variable
			node->left = new ASTNode;
			std::vector<Token> newTokens = SliceVector(tokens, 0, i);

			CreateAST(newTokens, node->left, node);
			if (HasError()) return false;

			// Parse the right side
			node->right = new ASTNode;
			newTokens = SliceVector(tokens, i + 1);

			CreateAST(newTokens, node->right, node);
			if (HasError()) return false;

			return true;
		}
	}

	return false;
}
