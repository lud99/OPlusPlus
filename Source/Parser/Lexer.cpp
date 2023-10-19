#include "Lexer.h"

#include <string>
#include <sstream>

namespace Ö
{
	bool IsValidVariablePart(std::string string, int index, std::string& error);

	std::string Token::ToString()
	{
		std::string names[] = {
			"Empty",
			"VoidType",
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

			"ClassKeyword",

			"SingleLineComment",
			"MultiLineComment",
			"NewLine",

			"Variable",
			
			"MemberAccessor",
			"ScopeResultion",

			"PropertyAccess",

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
			"Global"
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
				if (string[index + 1] == '.')
				{					
					error = "Expected a number after the decimal point, not " + std::string(1, string[index + 1]);
					return false;
				}					
				
				// Found a property access (or closing parenthesis), return with no error
				if (IsValidVariablePart(string, index - 1, error) || string[index - 1] == ')')
					return false;

				if (IsValidVariablePart(string, index + 1, error))
				{
					error = "Expected a number after the decimal point, not " + std::string(1, string[index + 1]);
					return false;
				}

				if (string[index - 1] == ' ' || string[index - 1] == '\n' || string[index - 1] == '-' || IsValidNumberPart(string, index - 1, error))
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

				// If previous is a digit, then that means a minus sign is inside a number, which is not allowed
				if (isdigit(string[index - 1]))
					return false;

				// If previous is a space or the previous is a variable
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

				// It is a valid number if the digit after the minus sign is valid
				// -5.4 => negative 5.4
				// - 5.4 => subtract 5.4
				if (IsValidNumberPart(string, index + 1, error))
				{
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
			if (current == ' ' || current == '\n')
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
		if (token.m_Value == "void")
			token.m_Type = Token::VoidType;
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

		else if (token.m_Value == "class")
			token.m_Type = Token::ClassKeyword;

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
		else if (token.m_Value == "global")
			token.m_Type = Token::Global;
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
		return m_ParenthesesParsingDepth + m_ScopeParsingDepth;
	}

	Token Lexer::AddToken(Token token, int customDepth)
	{
		if (token.m_Type == Token::Empty)
			return token;

		token.m_Depth = customDepth == -1 ? TotalDepth() : customDepth;

		m_Tokens.push_back(token);
		return Token(/*Token::Empty, token.m_StartPosition + 1*/);
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

		return std::to_string(m_CurrentLine + 1) + ":" + std::to_string(lineIndex + 1) + " " + message;
	}

	std::string Lexer::CreateTokens(const std::string& source, bool createCommentTokens)
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

		auto isInComment = [&] {
			return isInSingleLineComment || isMultilineComment;
		};

		auto shouldAddSemicolon = [&] {
			return !m_Tokens.empty() &&
				m_Tokens.back().m_Type != Token::Semicolon &&
				m_Tokens.back().m_Type != Token::NewLine &&
				m_Tokens.back().m_Type != Token::LeftCurlyBracket &&
				m_Tokens.back().m_Type != Token::RightCurlyBracket &&
				!isInComment();
		};

		Token token;
		for (m_Position = 0; m_Position < source.length(); Skip())
		{
			if (Current() == '\n')
			{
				m_CurrentLine++;

				if (isInSingleLineComment)
					isInSingleLineComment = false;

				if (shouldAddSemicolon())
					AddToken(Token(Token::Semicolon, m_Position, ";"));

				if (createCommentTokens)
					AddToken(Token(Token::NewLine, m_Position));
				
				continue;
			}

			std::string error;

			// Skip spaces
			if (!isInString && Current() == ' ')
			{
				if (isInComment())
				{
					if (createCommentTokens)
						token.m_Value += Current();
				}
				else
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
				token.m_StartPosition = m_Position;
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
						token = AddToken(token); // todo?
						//token = Token();
						token.m_StartPosition = m_Position;

						if (Next() == '*')
						{
							isMultilineComment = true;
							if (createCommentTokens) token.m_Type = Token::MultiLineComment;

							Skip();
							continue;
						}
						if (Next() == '/')
						{
							if (!isMultilineComment) isInSingleLineComment = true;

							if (createCommentTokens) token.m_Type = Token::SingleLineComment;

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
							if (createCommentTokens) AddToken(token);
							token = Token();
							Skip();
							continue;
						}
					}

					if (isInSingleLineComment && Current() == '\n')
					{
						isInSingleLineComment = false;
						if (createCommentTokens) AddToken(token);
						token = Token();
					}

					if (createCommentTokens)
						token.m_Value += Current();

					continue;
				}

				else if (Current() == '#')
				{
					token = AddToken(Token(Token::MemberAccessor, m_Position, "#", TotalDepth()));
				}

				// If parsing variable name (or a variable typeEntry, as long as it is a valid variable name)
				else if (IsValidVariablePart(source, m_Position, error))
				{
					token.m_StartPosition = m_Position;
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
					token.m_StartPosition = m_Position;
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
					token.m_StartPosition = m_Position;
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
					token.m_StartPosition = m_Position;
				}

				if (error != "") return MakeError(error);

				// Check for property access
				if (Current() == '.')
					AddToken(Token(Token::PropertyAccess, m_Position, "."));

				// Check for curly brackets
				if (Current() == '{')
				{
					m_ScopeParsingDepth++;
					AddToken(Token(Token::LeftCurlyBracket, m_Position, "{"));
				}
				else if (Current() == '}')
				{
					AddToken(Token(Token::RightCurlyBracket, m_Position, "}"));
					m_ScopeParsingDepth--;
				}
				// Check for parathesis
				if (Current() == '(')
				{
					// The variable in the previous token is actually a function call
					if (!m_Tokens.empty() && m_Tokens.back().m_Type == Token::Variable)
						m_Tokens.back().m_Type = Token::FunctionName;

					m_ParenthesesParsingDepth++;
					AddToken(Token(Token::LeftParentheses, m_Position, "("));
				}
				else if (Current() == ')')
				{
					AddToken(Token(Token::RightParentheses, m_Position, ")"));
					m_ParenthesesParsingDepth--;
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

				if (IsNext() && (Current() == ':' && Next() == ':'))
				{
					token = AddToken(Token(Token::ScopeResultion, m_Position, "::", TotalDepth()));
					Skip();
					continue;
				}

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
							AddToken(Token(Token::CompareEquals, m_Position, "=="));
							foundMatch = true;
						}
						// !=
						if (Current() == '!' && Next() == '=')
						{
							AddToken(Token(Token::NotEquals, m_Position, "!="));
							foundMatch = true;
						}
						// <=
						if (Current() == '<' && Next() == '=')
						{
							AddToken(Token(Token::LessThanEqual, m_Position, "<="));
							foundMatch = true;
						}
						// >=
						if (Current() == '>' && Next() == '=')
						{
							AddToken(Token(Token::GreaterThanEqual, m_Position, ">="));
							foundMatch = true;
						}
						// =>
						if (Current() == '=' && Next() == '>')
						{
							AddToken(Token(Token::RightArrow, m_Position, "=>"));
							foundMatch = true;
						}
					}

					// >
					if (Current() == '>' && !foundMatch)
					{
						AddToken(Token(Token::GreaterThan, m_Position, ">"));
						foundMatch = true;
					}
					if (Current() == '<' && !foundMatch)
					{
						AddToken(Token(Token::LessThan, m_Position, "<"));
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
							AddToken(Token(Token::PlusEquals, m_Position, "+="));
						else if (Current() == '-')
							AddToken(Token(Token::MinusEquals, m_Position, "-="));

						// Skip parsing the equals sign, or else there will be duplicates
						Skip();
						continue;
					}
				}

				// a++ or a--
				if (IsValidIncrement(m_Source, m_Position) || IsValidDecrement(m_Source, m_Position))
				{
					if (IsValidIncrement(m_Source, m_Position))
						AddToken(Token(Token::PostIncrement, m_Position, "++"));
					if (IsValidDecrement(m_Source, m_Position))
						AddToken(Token(Token::PostDecrement, m_Position, "--"));

					Skip();
					continue;
				}

				// Logical and
				if (IsNext() && (Current() == '&' && Next() == '&'))
				{
					token = AddToken(Token(Token::And, m_Position, "&&"));
					Skip();
					continue;
				}
				// Logical or
				if (IsNext() && (Current() == '|' && Next() == '|'))
				{
					token = AddToken(Token(Token::Or, m_Position, "||"));
					Skip();
					continue;
				}

				// Not
				if (Current() == '!')
					token = AddToken(Token(Token::Not, m_Position, "!"));
				// Xor
				if (Current() == '^')
					token = AddToken(Token(Token::Xor, m_Position, "^"));
				// Modulus
				if (Current() == '%')
					token = AddToken(Token(Token::Modulus, m_Position, "%"));

				// Plus
				if (Current() == '+')
					AddToken(Token(Token::Add, m_Position, "+"));
				// Subtract
				else if (Current() == '-')
					AddToken(Token(Token::Subtract, m_Position, "-"));
				// Multiply
				else if (Current() == '*')
					AddToken(Token(Token::Multiply, m_Position, "*"));
				// Divide
				else if (Current() == '/')
					AddToken(Token(Token::Divide, m_Position, "/"));

				// Equals sign. Make sure the last token wasn't a PlusEquals or similar, or else there will be duplicates
				else if (Current() == '=')// && (tokens.back().Type != Token::PlusEquals || tokens.back().Type != Token::MinusEquals))
					AddToken(Token(Token::SetEquals, m_Position, "="));

				//if (string[i] == ':')
				//{
				//	// Add the token with whatever content
				//	if (token.Type != Token::Null)
				//		AddToken(tokens, token);

				//	AddToken(tokens, Token::Colon, ":", totalBracketDepth());
				//}

				if (Current() == ',')
					AddToken(Token(Token::Comma, m_Position, ","));

				// Check for line ends
				if (Current() == ';')
					AddToken(Token(Token::Semicolon, m_Position, ";"));

				if (Current() == '\n')
				{
					m_CurrentLine++;

					isInSingleLineComment = false;

					if (shouldAddSemicolon())
						AddToken(Token(Token::Semicolon, m_Position, ";"));

					//if (createCommentTokens)
					//	AddToken(Token(Token::NewLine, m_Position));
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

		AddToken(token);

		// Unclosed comment
		if (isMultilineComment)
			return "Expected the comment to end";

		// Check for unclosed strings
		if (isInString)
			return "Expected the string to end";

		// Check for unclosed scopes
		if (m_ScopeParsingDepth != 0)
			return "Expected closing curly bracket";

		if (m_ParenthesesParsingDepth != 0)
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
}