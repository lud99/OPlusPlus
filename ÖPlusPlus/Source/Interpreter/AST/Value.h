#pragma once

#include "../ValueTypes.h"
#include "../../Parser.h"

#include <string>

namespace ASTint
{
	class Value
	{
	public:
		Value(ValueTypes type) : m_Type(type) {};

		Value(int value, ValueTypes type);
		Value(double value, ValueTypes type);
		Value(std::string value, ValueTypes type);

		std::string GetString();
		int GetInt();
		double GetFloat();

		inline bool IsString() { return m_Type == ValueTypes::String || m_Type == ValueTypes::StringReference || m_Type == ValueTypes::StringConstant; };
		bool IsTruthy();

		// Same as ToFormattedString, but without extra quotes for use internally
		std::string ToString();
		std::string ToFormattedString(bool extraQuotes = true, int currentDepth = 0);

		static bool IsSamePrimitiveType(Value lhs, Value rhs);
		static bool IsSamePrimitiveType(ValueTypes lhs, ValueTypes rhs);

		static bool Compare(Value lhs, Value rhs, ASTTypes comparisonType);
		static bool CompareEquals(Value lhs, Value rhs);
		static bool CompareNotEquals(Value lhs, Value rhs);
		static bool CompareLessThan(Value lhs, Value rhs);
		static bool CompareGreaterThan(Value lhs, Value rhs);
		static bool CompareLessThanEqual(Value lhs, Value rhs);
		static bool CompareGreaterThanEqual(Value lhs, Value rhs);

		static Value Add(Value& lhs, Value& rhs);
		static Value Subtract(Value& lhs, Value& rhs);
		static Value Divide(Value& lhs, Value& rhs);
		static Value Multiply(Value& lhs, Value& rhs);

	private:
		double m_FloatValue = 0.0;
		int m_IntValue = 0;
		std::string m_StringValue = "";

		ValueTypes m_Type = ValueTypes::Void;
	};
};