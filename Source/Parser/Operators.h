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
		// p = 2
		PostfixIncrement, PostfixDecrement,
		Call,
		Subscript,
		MemberAccess,

		// p = 3
		Closure,
		PrefixIncrement, PrefixDecrement,
		UnaryPlus, UnaryMinus,
		LogicalNot, BitwiseNot,

		// Binary
		// p = 5
		Multiplication, Division, Remainder,

		// p = 6
		Addition, Subtraction,
		
		// p = 9
		LessThan, LessThanOrEqual,
		GreaterThan, GreaterThanOrEqual,

		// p = 10
		Equality, NotEqual,
		
		// p = 16
		DirectAssignment,
		CompoundAssignmentSum, CompoundAssignmentDifference,
		CompoundAssignmentProduct, CompoundAssignmentQuotinent
	};

	enum Affix
	{
		Prefix,
		Midfix,
		Postfix
	};

	// Precedence is what decides what order the operators are executed in
	// Higher precedence means it executes first, so at the bottom of the AST Tree
	struct Operator
	{
		Name m_Name;
		std::string m_Symbol;

		Affix m_Affix; // The position of the operator in an expression
		Type m_Type;

		Token::Types m_TokenType;
		
		int m_Precedence;
		Associativity m_Associaticity;

		// Used for sorting the operators based on their precedence
		// Lower precedence comes first
		inline bool operator() (const Operator& left, const Operator& right)
		{
			return left.m_Precedence > right.m_Precedence;
		}

		// If the precedence is one less, then it is right associative. 
		// If its the same then it is left associative
		int GetParsePrecedence()
		{
			constexpr int HighestPrecedence = 17;
			// My precedence is specified as lowest being executed first, but parsed last.
			// The pratt parser is inverted so lowest means it is parsed first, and therefor executed last
			int invertedPrecedence = HighestPrecedence - m_Precedence + 1;
			return invertedPrecedence - (m_Associaticity == Operators::Associativity::Right ? 1 : 0);
		}

		std::string ToString() 
		{
			return std::string(magic_enum::enum_name(m_Type)) + " operator " + std::string(magic_enum::enum_name(m_Name));
		}
	};

	class DefinedOperators
	{
	public:
		void AddOperator(Name name, std::string m_Symbol, Affix affix, Type type, Token::Types tokenType, int precedence, Associativity associaticity)
		{ 
			m_Operators.push_back({ name, m_Symbol, affix, type, tokenType, precedence, associaticity });
			std::sort(m_Operators.begin(), m_Operators.end(), Operator());
		} 

		std::optional<Operator> GetUnaryPrefix(Token::Types type)
		{
			for (auto& op : m_Operators)
			{
				if (op.m_TokenType == type && op.m_Type == Unary && op.m_Affix == Prefix)
					return op;
			}
			return {};
		}
		std::optional<Operator> GetUnaryPostfix(Token::Types type)
		{
			for (auto& op : m_Operators)
			{
				if (op.m_TokenType == type && op.m_Type == Unary && op.m_Affix == Postfix)
					return op;
			}
			return {};
		}
		std::optional<Operator> GetBinary(Token::Types type)
		{
			for (auto& op : m_Operators)
			{
				if (op.m_TokenType == type && op.m_Type == Binary)
					return op;
			}
			return {};
		}
		std::optional<Operator> GetAny(Token::Types type)
		{
			for (auto& op : m_Operators)
			{
				if (op.m_TokenType == type)
					return op;
			}
			return {};
		}

		auto& GetOperators() { return m_Operators; }

	private:
		std::vector<Operator> m_Operators;
	};
}