#include "Lexer.h"

#include <string>
#include <sstream>

std::string Token::ToString()
{
	std::string names[] = {
		"Empty",
		"IntType",
		"IntLiteral",
		"FloatType",
		"FloatLiteral",
		"DoubleType",
		"DoubleLiteral",
		"StringType",
		"StringLiteral",
		"CharType",
		"CharLiteral",

		"Variable",

		"Semicolon",
		"Comma",
		"Colon",

		"Add",
		"Subtract",
		"Multiply",
		"Divide",
		"PlusEquals",
		"MinusEquals",

		"SetEquals",
		"CompareEquals",
		"NotEquals",
		"LessThan",
		"GreaterThan",
		"LessThanEqual",
		"GreaterThanEqual",

		"RightArrow",

		"And",
		"Or",
		"Not",
		"LeftShift",
		"RightShift",
		"Xor",
		"Modulus",

		"PostIncrement",
		"PreIncrement",
		"PostDecrement",
		"PreDecrement",

		"LeftParentheses",
		"RightParentheses",
		"LeftCurlyBracket",
		"RightCurlyBracket",
		"LeftSquareBracket",
		"RightSquareBracket",

		"FunctionName",

		"If",
		"Else",
		"While",
		"For",
		"Break",
		"Continue",
		"Return",
	};

	return names[(int)m_Type];
}

bool IsValidNumberChar(char ch)
{
	if (ch == ' ') return false;
	if (isdigit(ch) || ch == '.' || ch == '-')
		return true;

	return false;
}
bool IsValidVariableChar(char ch)
{
	if (ch == ' ') return false;
	if (isalpha(ch) || ch == '_')
		return true;
	return false;
}
bool IsValidNumberPart(const std::string& string, int index, std::string& error)
{
	// 5....1;
	char current = string[index];

	if (isdigit(current))
		return true;

	// At end or start
	if (index == 0 || index == string.length() - 1)
	{
		if (index == 0)
		{
			if (current == '-')
			{
				if (isdigit(string[index + 1]))
					return true;
				return false;
			}
			if (current == '.')
			{
				if (isdigit(string[index + 1]))
					return true;
				error = "Expected number after decimal point";
				return false;
			}
		}
		else
		{
			if (current == '.')
			{
				error = "Expected number after decimal point";
				return false;
			}
			if (current == '-')
			{
				error = "Expected number after minus sign";
				return false;
			}

			return isdigit(current);
		}
	}
	else
	{
		// Walk backwards and check if this variable part is actually valid
		if (isdigit(current))
			return true;

		if (current == '.')
		{
			if (string[index + 1] == '.' || !IsValidNumberPart(string, index + 1, error))
			{
				error = "Expected a number after the decimal point";
				return false;
			}

			if (string[index - 1] == ' ' || IsValidNumberPart(string, index - 1, error))
			{
				// Check for more decimal points
				for (int i = index - 1; i >= 0; i--)
				{
					// Don't check stuff that isn't part of the number
					if (!IsValidNumberChar(string[i]))
						break;

					if (string[i] == '.')
					{
						error = "Cannot have multiple decimal points in a number";
						return false;
					}
				}

				// Check for more decimal points
				for (int i = index + 1; i < string.length(); i++)
				{
					// Don't check stuff that isn't part of the number
					if (!IsValidNumberChar(string[i]))
						break;

					if (string[i] == '.')
					{
						error = "Cannot have multiple decimal points in a number";
						return false;
					}
				}

				return true;
			}
		}

		if (current == '-')
		{
			if (string[index + 1] == '-' || !IsValidNumberPart(string, index + 1, error))
			{
				return false;
			}

			if (string[index - 1] == ' ' || IsValidNumberPart(string, index - 1, error))
			{
				// Check for more minus signs
				for (int i = index - 1; i >= 0; i--)
				{
					// Don't check stuff that isn't part of the number
					if (!IsValidNumberChar(string[i]))
						break;

					if (string[i] == '-')
					{
						error = "Cannot have multiple minus signs in a number";
						return false;
					}
				}

				return true;
			}
		}
	}

	return false;
}
bool IsValidVariablePart(std::string string, int index, std::string& error)
{
	char current = string[index];

	// At end or start
	if (index == 0 || index == string.length() - 1)
	{
		if (IsValidVariableChar(current))
			return true;

		// Check if prev char exists, and if so if its a variable
		if (index > 0)
		{
			if (isdigit(current) && IsValidVariablePart(string, index - 1, error))
				return true;
		}
	}
	else
	{
		if (current == ' ')
			return false;

		if (IsValidVariableChar(current))
			return true;

		if (!IsValidVariablePart(string, index - 1, error) && !IsValidVariableChar(string[index + 1]))
			return false;

		if (isdigit(current) && IsValidVariablePart(string, index - 1, error))
			return true;

		if (isdigit(current))
			error = "Variable name cannot start with a number";
	}

	return false;
}

bool IsValidIncrement(const std::string& string, int index) 
{
	if (index < string.length() - 1)
		return (string[index] == '+' && string[index + 1] == '+');
	return false;
}

bool IsValidDecrement(const std::string& string, int index)
{
	if (index < string.length() - 1)
		return (string[index] == '-' && string[index + 1] == '-');
	return false;
}

std::vector<std::string> split(const std::string& txt, char ch)
{
	std::vector<std::string> strs;
	size_t pos = txt.find(ch);
	size_t initialPos = 0;
	strs.clear();

	// Decompose statement
	while (pos != std::string::npos) {
		strs.push_back(txt.substr(initialPos, pos - initialPos));
		initialPos = pos + 1;

		pos = txt.find(ch, initialPos);
	}

	// Add the last one
	strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

	return strs;
}

Token ResolveTokenKeyword(Token tok)
{
	Token token = tok;
	if (token.m_Value == "int")
		token.m_Type = Token::IntType;
	else if (token.m_Value == "float")
		token.m_Type = Token::FloatType;
	else if (token.m_Value == "double")
		token.m_Type = Token::DoubleType;
	else if (token.m_Value == "string")
		token.m_Type = Token::StringType;
	else if (token.m_Value == "char")
		token.m_Type = Token::CharType;

	else if (token.m_Value == "return")
		token.m_Type = Token::Return;
	else if (token.m_Value == "if")
		token.m_Type = Token::If;
	else if (token.m_Value == "else")
		token.m_Type = Token::Else;
	else if (token.m_Value == "while")
		token.m_Type = Token::While;
	else if (token.m_Value == "for")
		token.m_Type = Token::For;
	else if (token.m_Value == "continue")
		token.m_Type = Token::Continue;
	else if (token.m_Value == "break")
		token.m_Type = Token::Break;
	else if (token.m_Value == "and")
		token.m_Type = Token::And;
	else if (token.m_Value == "or")
		token.m_Type = Token::Or;
	else if (token.m_Value == "not")
		token.m_Type = Token::Not;
	else if (token.m_Value == "xor")
		token.m_Type = Token::Xor;
	else if (token.m_Value == "mod")
		token.m_Type = Token::Modulus;

	// Convert boolean tokens 'true' and 'false' to numbers
	if (token.m_Type == Token::Variable && (token.m_Value == "true" || token.m_Value == "false"))
	{
		if (token.m_Value == "true")
			token.m_Value = "1";
		else
			token.m_Value = "0";

		token.m_Type = Token::IntLiteral;
	}
	
	return token;
}

char Lexer::ConsumeNext()
{
	return '0';
}
char Lexer::Next()
{
	if (IsNext())
		return m_Source[m_Position + 1];

	return 0;
}
bool Lexer::IsNext()
{
	return CharactersLeft() > 1;
}
char Lexer::Previous()
{
	if (m_Position > 0)
		return m_Source[m_Position - 1];

	return 0;
}
char Lexer::Current()
{
	return m_Source[m_Position];
}
int Lexer::CharactersLeft()
{
	return m_Source.length() - m_Position;
}
int Lexer::Skip()
{
	m_Position++;
	return m_Position;
}

int Lexer::TotalDepth()
{
	return m_ParenthesisParsingDepth + m_ScopeParsingDepth;
}

Token Lexer::AddToken(Token token, int customDepth)
{
	if (token.m_Type == Token::Empty)
		return token;

	token.m_Depth = customDepth == -1 ? TotalDepth() : customDepth;

	m_Tokens.push_back(token);
	return Token();
}

std::string Lexer::MakeError(const std::string& message)
{
	int lineIndex = m_Position;
	for (int i = 0; i < m_CurrentLine; i++)
	{
		if (m_Lines[i].length() == 0)
			lineIndex--;
		else
			lineIndex -= m_Lines[i].length() + 1;
	}

	return std::to_string(m_CurrentLine) + ":" + std::to_string(lineIndex + 1) + " " + message;
}

std::string Lexer::CreateTokens(const std::string& source)
{
	m_Lines = split(source, '\n');
	for (int i = 0; i < m_Lines.size(); i++)
	{
		m_Lines[i] += '\n';
	}

	m_Source = source;

	bool isInSingleLineComment = false;
	bool isMultilineComment = false;
	bool isInString = false;

	int functionParsingDepth = 0;
	int parenthesisParsingDepth = 0;
	int statementParsingDepth = 0;

	auto shouldAddSemicolon = [&] {
		return !m_Tokens.empty() && 
			m_Tokens.back().m_Type != Token::Semicolon && 
			m_Tokens.back().m_Type != Token::LeftCurlyBracket && 
			m_Tokens.back().m_Type != Token::RightCurlyBracket;
	};

	auto isInComment = [&] {
		return isInSingleLineComment || isMultilineComment;
	};
	
	Token token;
	for (m_Position = 0; m_Position < source.length(); Skip())
	{
		if (Current() == '\n')
		{
			m_CurrentLine++;

			if (shouldAddSemicolon())
				AddToken(Token(Token::Semicolon, ";"));
		}
			
		std::string error;

		// Skip spaces
		if (!isInString && Current() == ' ')
		{
			// Check if current token is a variable, because it might be 'null', 'string' or 'number' but be considered a variable still
			token = AddToken(ResolveTokenKeyword(token));
			continue;
		}

		// Check for start of string
		if ((Current() == '\"' || Current() == '\'') && !isInString)
		{
			if (isInComment())
				continue;
			isInString = true;
			token.m_Type = Token::StringLiteral;
		}
		// Check for end of string
		else if ((Current() == '\"' || Current() == '\'') && isInString)
		{
			if (isInComment())
				continue;

			isInString = false;
			token = AddToken(token);
		}
		// Otherwise to normal parsing. The 'else' is required, or else the first (") would be considered a part of the string 
		else if (!isInString)
		{
			// Check for comments
			if (Current() == '/')
			{
				if (IsNext())
				{
					if (Next() == '*')
					{
						isMultilineComment = true;
						Skip();
						continue;
					}
					if (Next() == '/')
					{
						if (!isMultilineComment) isInSingleLineComment = true;
						Skip();
						continue;
					}
				}
			} 
			if (isInComment())
			{
				if (isMultilineComment)
				{
					if (IsNext() && Current() == '*' && Next() == '/')
					{
						isMultilineComment = false;
						Skip();
						continue;
					}
				}

				if (isInSingleLineComment && Current() == '\n')
					isInSingleLineComment = false;

				continue;
			}

			// If parsing variable name (or a variable type, as long as it is a valid variable name)
			else if (IsValidVariablePart(source, m_Position, error))
			{
				while (IsValidVariablePart(source, m_Position, error))
				{
					if (error != "") return MakeError(error);

					token.m_Value += Current();
					token.m_Type = Token::Variable;
					Skip();
				}

				if (error != "") return MakeError(error);

				int depth = TotalDepth();
				if (token.m_Value == "else")
					depth++;

				token = AddToken(ResolveTokenKeyword(token), depth);
			}

			else if (!m_Tokens.empty() && m_Tokens.back().IsStatementKeyword())
			{
				//scopeParsingDepth++;
				m_Tokens.back().m_Depth++;

				if (Current() == '(')
					statementParsingDepth++;
			}

			// Number
			else if (IsValidNumberPart(source, m_Position, error))
			{
				while (IsValidNumberPart(source, m_Position, error))
				{
					if (error != "") return MakeError(error);

					token.m_Value += Current();

					if (token.m_Value.find('.') != std::string::npos)
						token.m_Type = Token::DoubleLiteral;
					else
						token.m_Type = Token::IntLiteral;

					Skip();
				}
				if (error != "") return MakeError(error);

				token = AddToken(token);
			}

			if (error != "") return MakeError(error);

			// Check for curly brackets
			if (Current() == '{')
			{
				m_ScopeParsingDepth++;
				AddToken(Token(Token::LeftCurlyBracket));
			}
			else if (Current() == '}')
			{
				AddToken(Token(Token::RightCurlyBracket));
				m_ScopeParsingDepth--;
			}
			// Check for parathesis
			if (Current() == '(')
			{
				// The variable in the previous token is actually a function call
				if (!m_Tokens.empty() && m_Tokens.back().m_Type == Token::Variable)
					m_Tokens.back().m_Type = Token::FunctionName;

				m_ParenthesisParsingDepth++;
				AddToken(Token(Token::LeftParentheses, "("));
			} else if (Current() == ')')
			{
				AddToken(Token(Token::RightParentheses, ")"));
				m_ParenthesisParsingDepth--;
			}

			//// Check for square brackets
			//if (string[i] == '[')
			//{
			//	squareBracketsParsingDepth++;
			//	AddToken(tokens, Token::LeftSquareBracket, "[", totalBracketDepth());
			//}
			//else if (string[i] == ']')
			//{
			//	AddToken(tokens, Token::RightSquareBracket, "]", totalBracketDepth());
			//	squareBracketsParsingDepth--;
			//}
			//

			// Check for conditions. ==, !=, <, >, <=, >= 
			// TODO: seperate function with error checking
			{
				bool foundMatch = false;

				if (Current() == '=' && Previous() == '=')
					return MakeError("Can not have equals sign directly after another equals sign");

				if (IsNext())
				{
					// ==
					if (Current() == '=' && Next() == '=')
					{
						AddToken(Token(Token::CompareEquals, "=="));
						foundMatch = true;
					}
					// !=
					if (Current() == '!' && Next() == '=')
					{
						AddToken(Token(Token::NotEquals, "!="));
						foundMatch = true;
					}
					// <=
					if (Current() == '<' && Next() == '=')
					{
						AddToken(Token(Token::LessThanEqual, "<="));
						foundMatch = true;
					}
					// >=
					if (Current() == '>' && Next() == '=')
					{
						AddToken(Token(Token::GreaterThanEqual, ">="));
						foundMatch = true;
					}
					// =>
					if (Current() == '=' && Next() == '>')
					{
						AddToken(Token(Token::RightArrow, "=>"));
						foundMatch = true;
					}
				}

				// >
				if (Current() == '>' && !foundMatch)
				{
					AddToken(Token(Token::GreaterThan, ">"));
					foundMatch = true;
				}
				if (Current() == '<' && !foundMatch)
				{
					AddToken(Token(Token::LessThan, "<"));
					foundMatch = true;
				}

				if (foundMatch)
				{
					Skip();
					continue;
				}
			}

			// Check for plus equals, minus equals etc. ex: age += 2
			if (IsNext())
			{
				if (Next() == '=' && (Current() == '+' || Current() == '-'))
				{
					if (Current() == '+')
						AddToken(Token(Token::PlusEquals, "+="));
					else if (Current() == '-')
						AddToken(Token(Token::MinusEquals, "-="));

					// Skip parsing the equals sign, or else there will be duplicates
					Skip();
					continue;
				}
			}

			// a++ or a--
			if (IsValidIncrement(m_Source, m_Position) || IsValidDecrement(m_Source, m_Position))
			{
				if (IsValidIncrement(m_Source, m_Position))
					AddToken(Token(Token::PostIncrement, "++"));
				if (IsValidDecrement(m_Source, m_Position))
					AddToken(Token(Token::PostDecrement, "--"));

				Skip();
				continue;
			}

			// Logical and
			if (IsNext() && (Current() == '&' && Next() == '&'))
			{
				token = AddToken(Token(Token::And, "&&"));
				Skip();
				continue;
			}
			// Logical or
			if (IsNext() && (Current() == '|' && Next() == '|'))
			{
				token = AddToken(Token(Token::Or, "||"));
				Skip();
				continue;
			}

			// Not
			if (Current() == '!')
				token = AddToken(Token(Token::Not, "!"));
			// Xor
			if (Current() == '^')
				token = AddToken(Token(Token::Xor, "^"));
			// Modulus
			if (Current() == '%')
				token = AddToken(Token(Token::Modulus, "%"));

			// Plus
			if (Current() == '+')
				AddToken(Token(Token::Add, "+"));
			// Subtract
			else if (Current() == '-')
				AddToken(Token(Token::Subtract, "-"));
			// Multiply
			else if (Current() == '*')
				AddToken(Token(Token::Multiply, "*"));
			// Divide
			else if (Current() == '/')
				AddToken(Token(Token::Divide, "/"));

			// Equals sign. Make sure the last token wasn't a PlusEquals or similar, or else there will be duplicates
			else if (Current() == '=')// && (tokens.back().Type != Token::PlusEquals || tokens.back().Type != Token::MinusEquals))
				AddToken(Token(Token::SetEquals, "="));

			//if (string[i] == ':')
			//{
			//	// Add the token with whatever content
			//	if (token.Type != Token::Null)
			//		AddToken(tokens, token);

			//	AddToken(tokens, Token::Colon, ":", totalBracketDepth());
			//}

			if (Current() == ',')
				AddToken(Token(Token::Comma, ","));

			// Check for line ends
			if (Current() == ';')
				AddToken(Token(Token::Semicolon, ";"));

			if (Current() == '\n')
			{
				m_CurrentLine++;

				if (shouldAddSemicolon())
					AddToken(Token(Token::Semicolon, ";"));
			}

		}
		// Character in a string
		else
		{
			if (isInComment())
				continue;

			// Check for escaped characters
			if (Current() == '\\')
			{
				// If not the last
				if (CharactersLeft() >= 1)
				{
					// Todo: add more escapes perhaps
					if (Next() == 'n')
						token.m_Value += '\n';
					if (Next() == 'r')
						token.m_Value += '\r';
					else if (Next() == '\'')
						token.m_Value += '\'';
					else if (Next() == '\"')
						token.m_Value += '\"';
					else if (Next() == '\\')
						token.m_Value += '\\';
					Skip();
				}
			}
			else
			{
				token.m_Value += Current();
			}
		}
	}

	// Unclosed comment
	if (isInComment())
		return "Expected the comment to end";

	// Check for unclosed strings
	if (isInString)
		return "Expected the string to end";

	// Check for unclosed scopes
	if (m_ScopeParsingDepth != 0)
		return "Expected closing curly bracket";

	if (parenthesisParsingDepth != 0)
		return "Expected closing parenthesis";

	// Check if current token is a variable, because it might be 'null', 'string' or 'number' but be considered a variable still
	/*if (token.Type == Token::Variable)
	{
		if (token.Value == "null")
			token.Type = Token::NullType;
		else if (token.Value == "number")
			token.Type = Token::NumberVariableDeclaration;
		else if (token.Value == "string")
			token.Type = Token::StringVariableDeclaration;
	}

	if (token.Value != "")
		tokens.push_back(token);*/

	// Add a new line
	//tokens.push_back(Token(Token::EndStatment, "\n"));

	return "";
}
