#include "Lexer.h"

#include <string>
#include <sstream>
#include "../magic_enums.hpp"

namespace Ö::Lexer
{
	bool IsValidVariablePart(std::string string, int index, std::string& error);

	static std::string ReplaceTabsWithSpaces(std::string text, int tabSize = 4)
	{
		std::string newString = text;

		std::string tabReplacement = "";
		for (int i = 0; i < tabSize; i++)
			tabReplacement += " ";

		size_t position = 0;
		while ((position = newString.find("\t")) != std::string::npos)
			newString.replace(position, 1, tabReplacement);

		return newString;
	}

	std::string Token::TypeToString()
	{
		return TokenTypeToString(m_Type);
	}

	std::string Token::ToFormattedValueString()
	{
		if (m_Type == Token::StringLiteral)
			return "\"" + m_Value + "\"";

		return m_Value;
	}

	std::string TokenTypeToString(Token::Types type)
	{
		return std::string(magic_enum::enum_name(type));
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
					//error = "Expected number after minus sign";
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
		/*if (token.m_Value == "void")
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
			token.m_Type = Token::CharType;*/

		if (token.m_Value == "class")
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
		else if (token.m_Value == "loop")
			token.m_Type = Token::Loop;
		else if (token.m_Value == "continue")
			token.m_Type = Token::Continue;
		else if (token.m_Value == "break")
			token.m_Type = Token::Break;
		else if (token.m_Value == "closure")
			token.m_Type = Token::Closure;
		else if (token.m_Value == "global")
			token.m_Type = Token::Global;
		else if (token.m_Value == "and")
			token.m_Type = Token::And;
		else if (token.m_Value == "or")
			token.m_Type = Token::Or;
		else if (token.m_Value == "not")
			token.m_Type = Token::Not;
		else if (token.m_Value == "xor")
			token.m_Type = Token::Power;
		else if (token.m_Value == "mod")
			token.m_Type = Token::Remainder;

		// Convert boolean tokens 'true' and 'false' to numbers
		/*if (token.m_Type == Token::Identifier && (token.m_Value == "true" || token.m_Value == "false"))
		{
			if (token.m_Value == "true")
				token.m_Value = "1";
			else
				token.m_Value = "0";

			token.m_Type = Token::IntLiteral;
		}*/

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

	// pos: 20
	//0: 0-6
	//1: 7-14
	//2: 15-24

	TokenPosition Lexer::CalculateLineAndColumn()
	{
		int lineIndex = m_Position;
		for (int i = 0; i < m_CurrentLine; i++)
		{
			if (m_Lines[i].length() == 0)
				lineIndex--;
			else
				lineIndex -= m_Lines[i].length();
		}

		return { m_CurrentLine, lineIndex };
	}

	Token Lexer::AddToken(Token token)
	{
		if (token.m_Type == Token::Empty)
			return token;

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

		return std::to_string(m_CurrentLine + 1) + ":" + std::to_string(lineIndex + 1) + " " + message;
	}

	std::string Lexer::CreateTokens(const std::string& source, bool createCommentTokens)
	{
		// Replace tabs with spaces to avoid annoying issues with lexing and source maps
		std::string noTabsSource = ReplaceTabsWithSpaces(source);

		m_Lines = split(noTabsSource, '\n');
		for (int i = 0; i < m_Lines.size(); i++)
		{
			m_Lines[i] += '\n';
		}

		m_Source = noTabsSource;

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
			return false;/* !m_Tokens.empty() &&
				m_Tokens.back().m_Type != Token::Semicolon &&
				m_Tokens.back().m_Type != Token::NewLine &&
				m_Tokens.back().m_Type != Token::LeftCurlyBracket &&
				m_Tokens.back().m_Type != Token::RightCurlyBracket &&
				!isInComment();*/
		};

		Token token;
		for (m_Position = 0; m_Position < m_Source.length(); Skip())
		{
			if (Current() == '\n')
			{
				m_CurrentLine++;

				if (isInSingleLineComment)
					isInSingleLineComment = false;

				if (shouldAddSemicolon())
					AddToken(Token(Token::Semicolon, CalculateLineAndColumn(), ";"));

				if (createCommentTokens)
					AddToken(Token(Token::NewLine, CalculateLineAndColumn()));
				
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
				token.m_StartPosition = CalculateLineAndColumn();
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
						token.m_StartPosition = CalculateLineAndColumn();

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
					token = AddToken(Token(Token::MemberAccessor, CalculateLineAndColumn(), "#"));
				}

				// If parsing variable name (or a variable typeEntry, as long as it is a valid variable name)
				else if (IsValidVariablePart(m_Source, m_Position, error))
				{
					token.m_StartPosition = CalculateLineAndColumn();
					while (IsValidVariablePart(m_Source, m_Position, error))
					{
						if (error != "") return MakeError(error);

						token.m_Value += Current();
						token.m_Type = Token::Identifier;
						Skip();
					}

					if (error != "") return MakeError(error);

					token = AddToken(ResolveTokenKeyword(token));
					token.m_StartPosition = CalculateLineAndColumn();
				}

				// Number
				else if (IsValidNumberPart(m_Source, m_Position, error))
				{
					token.m_StartPosition = CalculateLineAndColumn();
					while (IsValidNumberPart(m_Source, m_Position, error))
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
					token.m_StartPosition = CalculateLineAndColumn();
				}

				if (error != "") return MakeError(error);

				// Check for property access
				if (Current() == '.')
					AddToken(Token(Token::PropertyAccess, CalculateLineAndColumn(), "."));

				// Check for curly brackets
				if (Current() == '{')
				{
					AddToken(Token(Token::LeftCurlyBracket, CalculateLineAndColumn(), "{"));
				}
				else if (Current() == '}')
				{
					AddToken(Token(Token::RightCurlyBracket, CalculateLineAndColumn(), "}"));
				}
				// Check for parathesis
				if (Current() == '(')
				{
					AddToken(Token(Token::LeftParentheses, CalculateLineAndColumn(), "("));
				}
				else if (Current() == ')')
				{
					AddToken(Token(Token::RightParentheses, CalculateLineAndColumn(), ")"));
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
					token = AddToken(Token(Token::ScopeResultion, CalculateLineAndColumn(), "::"));
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
							AddToken(Token(Token::Equality, CalculateLineAndColumn(), "=="));
							foundMatch = true;
						}
						// !=
						if (Current() == '!' && Next() == '=')
						{
							AddToken(Token(Token::NotEqual, CalculateLineAndColumn(), "!="));
							foundMatch = true;
						}
						// <=
						if (Current() == '<' && Next() == '=')
						{
							AddToken(Token(Token::LessThanOrEqual, CalculateLineAndColumn(), "<="));
							foundMatch = true;
						}
						// >=
						if (Current() == '>' && Next() == '=')
						{
							AddToken(Token(Token::GreaterThanOrEqual, CalculateLineAndColumn(), ">="));
							foundMatch = true;
						}
						// =>
						if (Current() == '=' && Next() == '>')
						{
							AddToken(Token(Token::RightArrow, CalculateLineAndColumn(), "=>"));
							foundMatch = true;
						}
					}

					// >
					if (Current() == '>' && !foundMatch)
					{
						AddToken(Token(Token::GreaterThan, CalculateLineAndColumn(), ">"));
						foundMatch = true;
					}
					if (Current() == '<' && !foundMatch)
					{
						AddToken(Token(Token::LessThan, CalculateLineAndColumn(), "<"));
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
							AddToken(Token(Token::PlusEquals, CalculateLineAndColumn(), "+="));
						else if (Current() == '-')
							AddToken(Token(Token::MinusEquals, CalculateLineAndColumn(), "-="));

						// Skip parsing the equals sign, or else there will be duplicates
						Skip();
						continue;
					}
				}

				// a++ or a--
				if (IsValidIncrement(m_Source, m_Position) || IsValidDecrement(m_Source, m_Position))
				{
					if (IsValidIncrement(m_Source, m_Position))
						AddToken(Token(Token::Increment, CalculateLineAndColumn(), "++"));
					if (IsValidDecrement(m_Source, m_Position))
						AddToken(Token(Token::Decrement, CalculateLineAndColumn(), "--"));

					Skip();
					continue;
				}

				// Logical and
				if (IsNext() && (Current() == '&' && Next() == '&'))
				{
					token = AddToken(Token(Token::And, CalculateLineAndColumn(), "&&"));
					Skip();
					continue;
				}
				// Logical or
				if (IsNext() && (Current() == '|' && Next() == '|'))
				{
					token = AddToken(Token(Token::Or, CalculateLineAndColumn(), "||"));
					Skip();
					continue;
				}

				// Not
				if (Current() == '!')
					token = AddToken(Token(Token::Not, CalculateLineAndColumn(), "!"));
				// Power
				if (Current() == '^')
					token = AddToken(Token(Token::Power, CalculateLineAndColumn(), "^"));
				// Remainder
				if (Current() == '%')
					token = AddToken(Token(Token::Remainder, CalculateLineAndColumn(), "%"));

				// Plus
				if (Current() == '+')
					AddToken(Token(Token::Add, CalculateLineAndColumn(), "+"));
				// Subtract
				else if (Current() == '-')
					AddToken(Token(Token::Subtract, CalculateLineAndColumn(), "-"));
				// Multiply
				else if (Current() == '*')
					AddToken(Token(Token::Multiply, CalculateLineAndColumn(), "*"));
				// Divide
				else if (Current() == '/')
					AddToken(Token(Token::Divide, CalculateLineAndColumn(), "/"));

				// Equals sign. Make sure the last token wasn't a PlusEquals or similar, or else there will be duplicates
				else if (Current() == '=')// && (tokens.back().Type != Token::PlusEquals || tokens.back().Type != Token::MinusEquals))
					AddToken(Token(Token::SetEquals, CalculateLineAndColumn(), "="));

				//if (string[i] == ':')
				//{
				//	// Add the token with whatever content
				//	if (token.Type != Token::Null)
				//		AddToken(tokens, token);

				//	AddToken(tokens, Token::Colon, ":", totalBracketDepth());
				//}

				if (Current() == ',')
					AddToken(Token(Token::Comma, CalculateLineAndColumn(), ","));

				// Check for line ends
				if (Current() == ';')
					AddToken(Token(Token::Semicolon, CalculateLineAndColumn(), ";"));

				if (Current() == '\n')
				{
					m_CurrentLine++;

					isInSingleLineComment = false;

					if (shouldAddSemicolon())
						AddToken(Token(Token::Semicolon, CalculateLineAndColumn(), ";"));

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

		// Add eof token
		AddToken(Token(Token::Types::EndOfFile, CalculateLineAndColumn()));

		// Unclosed comment
		if (isMultilineComment)
			return "Expected the comment to end";

		// Check for unclosed strings
		if (isInString)
			return "Expected the string to end";

		return "";
	}

	std::string Lexer::ReconstructSourcecode(Tokens& tokens)
	{
		std::string sourceCode = "";

		int previousColumnEnd = 0;
		int previousLine = 0;
		for (auto& token : tokens)
		{
			int columnEnd = token.m_StartPosition.column + token.ToFormattedValueString().length();
			int line = token.m_StartPosition.line;

			int columnsBetween = 0;
			if (line == previousLine)
				columnsBetween = token.m_StartPosition.column - previousColumnEnd; // offset between tokens on same line
			else
				columnsBetween = token.m_StartPosition.column; // offset from start of line

			int linesBetween = token.m_StartPosition.line - previousLine;

			std::string space = "";
			for (int i = 0; i < columnsBetween; i++)
				space += " ";

			std::string newlines = "";
			for (int i = 0; i < linesBetween; i++)
				newlines += "\n";

			sourceCode += newlines + space + token.ToFormattedValueString();

			previousColumnEnd = columnEnd;
			previousLine = line;
		}

		return sourceCode;
	}
}