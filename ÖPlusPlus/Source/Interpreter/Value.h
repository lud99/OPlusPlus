#pragma once

#include <string>
#include <map>

#include "ValueTypes.h"
#include "../Parser.h"

#ifdef BYTECODE_INTERPRETER
#include "Bytecode/Heap.h"
#endif

class Value
{
public:
	Value();
	Value(ValueTypes type) : m_Type(type) {};

	Value(int value, ValueTypes type);
	Value(double value, ValueTypes type);
	Value(std::string value, ValueTypes type);
#ifdef BYTECODE_INTERPRETER
	Value(HeapEntry& value);
	Value(HeapEntry& value, ValueTypes type) : m_HeapEntryPointer(&value), m_Type(type) {};
#endif

	std::string GetString();
	int& GetInt();
	double& GetFloat();

	void SetString(std::string value);
	void SetInt(int value);
	void SetFloat(double value);

	ValueTypes GetType();
	void SetType(ValueTypes type);

	inline bool IsString() { return m_Type == ValueTypes::String || m_Type == ValueTypes::StringReference || m_Type == ValueTypes::StringConstant; };
	bool IsTruthy();

	// Same as ToFormattedString, but without extra quotes for use internally
	std::string ToString();
	std::string ToFormattedString(bool extraQuotes = true, int currentDepth = 0);

	void Delete();

	static Value MakeRuntimeError(std::string error);
	static bool MakeRuntimeErrorBool(std::string error);

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

	~Value();

public:
	enum class Flags
	{
		None = 0,
		LocalVariable,
		GlobalVariable
	};

public:
	std::string m_DebugInfo = "";

	Flags m_Flag = Flags::None;

#ifdef BYTECODE_INTERPRETER
	HeapEntry* m_HeapEntryPointer = nullptr;
#endif
#ifdef AST_INTERPRETER
	std::string m_Name = "";
#endif

private:
	double m_FloatValue = 0.0;
	int m_IntValue = 0;
	std::string m_StringValue = "";

	ValueTypes m_Type = ValueTypes::Void;
};

typedef std::vector<Value> ValueArray;
typedef std::map<std::string, Value> ObjectInstance;