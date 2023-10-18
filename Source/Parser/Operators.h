#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include "../magic_enums.hpp"

#include "Lexer.h"	

namespace Ö::Operators
{
	enum Associativity
	{
		Left,
		Right
	};
	enum Type
	{
		Unary,
		Binary
	};
	enum Name
	{
		// Unary
		LogicalNegation = '!',

		// Binary
		Addition = '+',
		Subtraction = '-',
		Multiplication = '*',
		Division = '/'
	};

	// Precedence is what decides what order the operators are executed in
	// Higher precedence means it executes first, so at the bottom of the AST Tree
	struct Operator
	{
		Name m_Name;
		Type m_Type;

		Token::Types m_TokenType;
		
		int m_Precedence;
		Associativity m_Associaticity;

		// Used for sorting the operators based on their precedence
		// Higher precedence comes first and reduces
		inline bool operator() (const Operator& left, const Operator& right)
		{
			return left.m_Precedence < right.m_Precedence;
		}

		std::string ToString() 
		{
			return std::string(magic_enum::enum_name(m_Type)) + " operator " + std::string(magic_enum::enum_name(m_Name));
		}
		std::string OnlySymbolToString()
		{
			return std::string(1, (char)m_Name);
		}
	};

	class DefinedOperators
	{
	public:
		void AddOperator(Name name, Type type, Token::Types tokenType, int precedence, Associativity associaticity)
		{ 
			m_Operators.push_back({ name, type, tokenType, precedence, associaticity });
			std::sort(m_Operators.begin(), m_Operators.end(), Operator());
		} 

		auto& GetOperators() { return m_Operators; }

	private:
		std::vector<Operator> m_Operators;
	};
}