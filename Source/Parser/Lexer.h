#pragma once

#include "../macro.h"

#include <string>
#include <vector>

namespace Ö
{
	struct Token
	{
		enum Types {
			Empty,
			IntLiteral,
			FloatLiteral,
			DoubleLiteral,
			StringLiteral,
			CharLiteral,

			ClassKeyword,

			SingleLineComment,
			MultiLineComment,
			NewLine,

			Identifier,

			MemberAccessor,
			ScopeResultion,

			PropertyAccess,

			Semicolon,
			Comma,
			Colon,

			Add,
			Subtract,
			Multiply,
			Divide,
			PlusEquals,
			MinusEquals,

			SetEquals,
			Equality,
			NotEqual,
			LessThan,
			GreaterThan,
			LessThanOrEqual,
			GreaterThanOrEqual,

			RightArrow,

			And,
			Or,
			Not,
			LeftShift,
			RightShift,
			Power,
			Remainder,

			Increment,
			Decrement,

			LeftParentheses,
			RightParentheses,
			LeftCurlyBracket,
			RightCurlyBracket,
			LeftSquareBracket,
			RightSquareBracket,

			If,
			Else,
			While,
			For,
			Loop,

			Closure,

			Break,
			Continue,

			Return,
			Global,

			EndOfFile,
		};

		Token() {};
		Token(Types type, int position, std::string value = "", int depth = -1) : m_Type(type), m_StartPosition(position), m_Value(value), m_Depth(depth) {};

		EXPORT std::string ToString();

		inline bool IsOperator()
		{
			Token::Types ops[12] = {
				Add,
				Subtract,
				Divide,
				Multiply,
				Not,
				Increment,
				Decrement,
				MemberAccessor,
				Remainder,
				SetEquals,
				PlusEquals,
				MinusEquals,
			};

			for (auto& type : ops)
			{
				if (m_Type == type)
					return true;
			}

			return IsComparisonOperator();
		}

		inline bool IsStatementKeyword()
		{
			return m_Type == Token::If || m_Type == Token::While || m_Type == Token::For;
		}

		inline bool IsComparisonOperator()
		{
			return m_Type == Token::Equality || m_Type == Token::NotEqual || m_Type == Token::LessThan || m_Type == Token::GreaterThan
				|| m_Type == Token::LessThanOrEqual || m_Type == Token::GreaterThanOrEqual;
		}

		inline bool IsLiteral()
		{
			return m_Type == Token::IntLiteral || m_Type == Token::DoubleLiteral 
				|| m_Type == Token::FloatLiteral || m_Type == Token::StringLiteral;
		}

		Types m_Type = Types::Empty;
		std::string m_Value;

		int m_StartPosition = 0;

		int m_Depth = -1;
	};

	std::string TokenTypeToString(Token::Types type);

	typedef std::vector<Token> Tokens;

	class Lexer
	{
	public:
		EXPORT std::string CreateTokens(const std::string& source, bool createCommentTokens = false);

		char ConsumeNext();
		char Next();
		bool IsNext();
		char Previous();
		char Current();
		int CharactersLeft();
		int Skip();

		int TotalDepth();

		Tokens& GetTokens() { return m_Tokens; };

		Token AddToken(Token token, int customDepth = -1);

		std::string MakeError(const std::string& message);

	private:
		std::vector<Token> m_Tokens;

		int m_Position = 0;
		int m_CurrentLine = 0;

		std::vector<std::string> m_Lines;
		std::string m_Source = "";

		int m_ParenthesesParsingDepth = 0;
		int m_ScopeParsingDepth = 0;
	};

};