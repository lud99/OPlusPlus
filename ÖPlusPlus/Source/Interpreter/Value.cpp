#include "Value.h"

#include <iostream>
#include <assert.h>

#ifdef AST_INTERPRETER
#include "AST/ASTInterpreter.h"
#endif
#ifdef BYTECODE_INTERPRETER
#include "Bytecode/BytecodeInterpreter.h"
#endif


#include "../Utils.hpp"

Value Value::MakeRuntimeError(std::string error)
{
#ifdef AST_INTERPRETER
	return ASTint::ASTInterpreter::Get().MakeErrorValueReturn(error);
#endif // AST_INTERPRETER
#ifdef BYTECODE_INTERPRETER
	return BytecodeInterpreter::Get().ThrowExceptionValue(error);
#endif // BYTECODE_INTERPRETER

}

bool Value::MakeRuntimeErrorBool(std::string error)
{
#ifdef AST_INTERPRETER
	ASTint::ASTInterpreter::Get().MakeErrorValueReturn(error);
#endif // AST_INTERPRETER
#ifdef BYTECODE_INTERPRETER
	BytecodeInterpreter::Get().ThrowExceptionValue(error);
#endif // BYTECODE_INTERPRETER
	return false;
}

Value::Value()
{
}


Value::Value(int value, ValueTypes type)
{
	assert(type == ValueTypes::Integer);
	m_Type = type;
	m_IntValue = value;
}
Value::Value(double value, ValueTypes type)
{
	assert(type == ValueTypes::Float);
	m_Type = type;
	m_FloatValue = value;
}
Value::Value(std::string value, ValueTypes type)
{
	m_Type = type;

	assert(IsString());

	m_StringValue = value;
}
#ifdef BYTECODE_INTERPRETER
Value::Value(HeapEntry& value)
{
	m_HeapEntryPointer = &value;

	if (value.m_Type == 0)
		m_Type = ValueTypes::StringReference;
	/*else if (value.m_Type == 1)
		m_Type = Value::Array;
	else if (value.m_Type == 2)
		m_Type = Value::Object;*/
}
#endif
std::string Value::GetString()
{
	assert(IsString());

#ifdef BYTECODE_INTERPRETER
	if (m_HeapEntryPointer != nullptr)
		return (char*)(m_HeapEntryPointer)->m_Data;
#endif // BYTECODE_INTERPRETER

	return m_StringValue;
}

int& Value::GetInt()
{
	assert(m_Type == ValueTypes::Integer);

	return m_IntValue;
}

double& Value::GetFloat()
{
	assert(m_Type == ValueTypes::Float);

	return m_FloatValue;
}

void Value::SetString(std::string value)
{
	assert(IsString());

	// TODO: might cause problems
	m_StringValue = value;

#ifdef BYTECODE_INTERPRETER
	if (m_HeapEntryPointer != nullptr)
		return (char*)(m_HeapEntryPointer)->m_Data;
#endif // BYTECODE_INTERPRETER
}

void Value::SetInt(int value)
{
	assert(m_Type == ValueTypes::Integer);

	m_IntValue = value;
}

void Value::SetFloat(double value)
{
	assert(m_Type == ValueTypes::Float);

	m_FloatValue = value;
}

ValueTypes Value::GetType()
{
	return m_Type;
}

void Value::SetType(ValueTypes type)
{
	m_Type = type;
}

std::string Value::ToString()
{
	return ToFormattedString(false);
}

std::string Value::ToFormattedString(bool extraQuotes, int currentDepth)
{
	if (m_Type == ValueTypes::Void)
		return "(void)";

	if (m_Type == ValueTypes::Integer)
	{
		return std::to_string(m_IntValue);
	}
	if (m_Type == ValueTypes::Float)
	{
		return std::to_string(m_FloatValue);
	}

	if (IsString())
	{
		if (extraQuotes)
			return "\"" + std::string(GetString()) + "\"";

		return GetString();
	}

	/*if (IsArray())
	{
		std::string out = "[";
		ValueArray* arr = GetArray();
		for (size_t i = 0; i < arr->size(); i++)
		{
			out += arr->at(i).ToFormattedString(true);
			if (i < arr->size() - 1)
				out += ", ";
		}

		out += "]";
		return out;
	}

	if (m_Type == Value::Object)
	{
		ObjectInstance& obj = *GetObjectInstance();
		std::string out = "{ ";
		int i = 0;

		int maxDepth = 3;

		for (auto& it : obj)
		{
			if (currentDepth >= maxDepth)
			{
				out += "...";
				break;
			}

			if (it.second.m_Type == Value::Object)
				currentDepth++;

			out += it.first + " = " + it.second.ToFormattedString(true, currentDepth);

			if (it.second.m_Type == Value::Object)
				currentDepth--;

			if (i != obj.size() - 1)
				out += ", ";

			i++;
		}

		out += "} ";
		return out;
	}

	if (m_Type == Value::Function)
	{
		return "{code}";
	}

	if (m_Type == Value::FunctionPointer)
	{
		return "&" + std::to_string((int)m_Value);
	}*/

	abort();

	return "unhandled type";
}

void Value::Delete()
{
#ifdef BYTECODE_INTERPRETER
	if (IsString())
	{
		// Delete the string, because strings aren't stored as a reference to each other. Don't delete constants!!!
		if (m_Type == ValueTypes::StringReference)
			BytecodeInterpreter::Get().m_Heap.DeleteObject(m_HeapEntryPointer);
	}
#endif
	
	m_Type = ValueTypes::Void;
}

bool Value::IsTruthy()
{
	assert(m_Type != ValueTypes::Void);

	if (IsString())
		return std::string(GetString()) != "";

	if (m_Type == ValueTypes::Integer)
		return GetInt() > 0;
	if (m_Type == ValueTypes::Float)
		return GetFloat() > 0;

	abort();
	return false;
}

bool Value::IsSamePrimitiveType(Value lhs, Value rhs)
{
	if (lhs.IsString() && rhs.IsString())
		return true;

	if (lhs.m_Type == rhs.m_Type)
		return true;

	// TODO: float = int

	return false;
}

bool Value::IsSamePrimitiveType(ValueTypes lhs, ValueTypes rhs)
{
	return IsSamePrimitiveType(Value(lhs), Value(rhs));
}

bool Value::Compare(Value lhs, Value rhs, ASTTypes comparisonType)
{
	if (comparisonType == ASTTypes::CompareEquals)
		return CompareEquals(lhs, rhs);
	if (comparisonType == ASTTypes::CompareNotEquals)
		return CompareNotEquals(lhs, rhs);
	if (comparisonType == ASTTypes::CompareLessThan)
		return CompareLessThan(lhs, rhs);
	if (comparisonType == ASTTypes::CompareGreaterThan)
		return CompareGreaterThan(lhs, rhs);
	if (comparisonType == ASTTypes::CompareLessThanEqual)
		return CompareLessThanEqual(lhs, rhs);
	if (comparisonType == ASTTypes::CompareGreaterThanEqual)
		return CompareGreaterThanEqual(lhs, rhs);

	return false;
}

//void Value::Delete()
//{
//	if (IsString())
//	{
//		// Delete the string, because strings aren't stored as a reference to each other. Don't delete constants!!!
//		if (m_Type == ValueTypes::StringReference)
//			BytecodeInterpreter::Get().m_Heap.DeleteObject(m_HeapEntryPointer);
//	}

//	m_ValueInt = 0;
//	m_Type = ValueTypes::Void;
//}


bool Value::CompareEquals(Value lhs, Value rhs)
{
	if (!Value::IsSamePrimitiveType(lhs, rhs)) 
		return MakeRuntimeErrorBool("Cannot compare type " + ValueTypeToString(lhs.GetType()) + " with " + ValueTypeToString(rhs.GetType()));

	ValueTypes& type = lhs.m_Type;

	// Numeric
	if (type == ValueTypes::Integer)
		return lhs.GetInt() == rhs.GetInt();
	if (type == ValueTypes::Float)
		return lhs.GetFloat() == rhs.GetFloat();

	// String reference
	if (lhs.IsString())
		return lhs.GetString() == rhs.GetString();

	abort();
	return false;
}


bool Value::CompareNotEquals(Value lhs, Value rhs)
{
	if (!Value::IsSamePrimitiveType(lhs, rhs))
		return MakeRuntimeErrorBool("Cannot compare type " + ValueTypeToString(lhs.GetType()) + " with " + ValueTypeToString(rhs.GetType()));

	ValueTypes& type = lhs.m_Type;

	// Numeric
	if (type == ValueTypes::Integer)
		return lhs.GetInt() != rhs.GetInt();
	if (type == ValueTypes::Float)
		return lhs.GetFloat() != rhs.GetFloat();

	// String reference
	if (lhs.IsString())
		return lhs.GetString() != rhs.GetString();

	abort();
	return false;
}

bool Value::CompareLessThan(Value lhs, Value rhs)
{
	if (!Value::IsSamePrimitiveType(lhs, rhs))
		return MakeRuntimeErrorBool("Cannot compare type " + ValueTypeToString(lhs.GetType()) + " with " + ValueTypeToString(rhs.GetType()));

	ValueTypes& type = lhs.m_Type;

	// Numeric
	if (type == ValueTypes::Integer)
		return lhs.GetInt() < rhs.GetInt();
	if (type == ValueTypes::Float)
		return lhs.GetFloat() < rhs.GetFloat();

	// String reference
	if (lhs.IsString())
		return std::string(lhs.GetString()).length() < std::string(rhs.GetString()).length();

	abort();
	return false;
}

bool Value::CompareGreaterThan(Value lhs, Value rhs)
{
	if (!Value::IsSamePrimitiveType(lhs, rhs))
		return MakeRuntimeErrorBool("Cannot compare type " + ValueTypeToString(lhs.GetType()) + " with " + ValueTypeToString(rhs.GetType()));

	ValueTypes& type = lhs.m_Type;

	// Numeric
	if (type == ValueTypes::Integer)
		return lhs.GetInt() > rhs.GetInt();
	if (type == ValueTypes::Float)
		return lhs.GetFloat() > rhs.GetFloat();

	// String reference
	if (lhs.IsString())
		return std::string(lhs.GetString()).length() > std::string(rhs.GetString()).length();

	abort();
	return false;
}

bool Value::CompareLessThanEqual(Value lhs, Value rhs)
{
	if (!Value::IsSamePrimitiveType(lhs, rhs))
		return MakeRuntimeErrorBool("Cannot compare type " + ValueTypeToString(lhs.GetType()) + " with " + ValueTypeToString(rhs.GetType()));

	ValueTypes& type = lhs.m_Type;

	// If both left and right are null
	/*if (type == Value::Null)
		return false;*/

	if (type == ValueTypes::Integer)
		return lhs.GetInt() <= rhs.GetInt();
	if (type == ValueTypes::Float)
		return lhs.GetFloat() <= rhs.GetFloat();

	// String reference
	if (lhs.IsString())
		return std::string(lhs.GetString()).length() <= std::string(rhs.GetString()).length();

	abort();
	return false;
}

bool Value::CompareGreaterThanEqual(Value lhs, Value rhs)
{
	if (!Value::IsSamePrimitiveType(lhs, rhs))
		return MakeRuntimeErrorBool("Cannot compare type " + ValueTypeToString(lhs.GetType()) + " with " + ValueTypeToString(rhs.GetType()));

	ValueTypes& type = lhs.m_Type;

	// If both left and right are null
	/*if (type == Value::Null)
		return false;*/

	if (type == ValueTypes::Integer)
		return lhs.GetInt() >= rhs.GetInt();
	if (type == ValueTypes::Float)
		return lhs.GetFloat() >= rhs.GetFloat();

	// String reference
	if (lhs.IsString())
		return std::string(lhs.GetString()).length() >= std::string(rhs.GetString()).length();

	abort();
	return false;
}

Value Value::Increment(Value& value)
{
	if (value.m_Type == ValueTypes::Integer)
	{
		return Value(value.GetInt() + 1, ValueTypes::Integer);
	}
	else if (value.m_Type == ValueTypes::Float)
	{
		return Value(value.GetFloat() + 1.0, ValueTypes::Float);
	}
#ifdef AST_INTERPRETER
	else if (value.IsString())
	{
		char lastChar = value.GetString()[value.GetString().length() - 1];
		std::string lastCharStr = std::string(1, lastChar);
		std::string appended = value.GetString() + lastCharStr;
		
		// Allocate a new object in the heap that stores the string
		//HeapEntry& newObject = BytecodeInterpreter::Get().m_Heap.CreateString(appended);

		return Value(appended, ValueTypes::String);
	}
#endif

	return MakeRuntimeError("Unhandled increment of type " + ValueTypeToString(value.m_Type));
}

Value Value::Decrement(Value& value)
{
	if (value.m_Type == ValueTypes::Integer)
	{
		return Value(value.GetInt() - 1, ValueTypes::Integer);
	}
	else if (value.m_Type == ValueTypes::Float)
	{
		return Value(value.GetFloat() - 1.0, ValueTypes::Float);
	}
#ifdef AST_INTERPRETER
	else if (value.IsString())
	{
		std::string removed = value.GetString().substr(0, value.GetString().length() - 1);

		// Allocate a new object in the heap that stores the string
		//HeapEntry& newObject = BytecodeInterpreter::Get().m_Heap.CreateString(appended);

		return Value(removed, ValueTypes::String);
	}
#endif

	return MakeRuntimeError("Unhandled decrement of type " + ValueTypeToString(value.m_Type));
}

Value Value::Add(Value& lhs, Value& rhs)
{
	if (!Value::IsSamePrimitiveType(lhs, rhs))
		return MakeRuntimeError("Cannot add types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));

	if (lhs.m_Type == ValueTypes::Integer)
	{
		return Value(lhs.GetInt() + rhs.GetInt(), ValueTypes::Integer);
	}
	else if (lhs.m_Type == ValueTypes::Float)
	{
		return Value(lhs.GetFloat() + rhs.GetFloat(), ValueTypes::Float);
	}
	else if (lhs.IsString() || rhs.IsString())
	{
		std::string appended;
		if (lhs.IsString() && rhs.IsString())
			appended = std::string(lhs.GetString()) + std::string(rhs.GetString());
		/*else if (lhs.IsString() && rhs.IsChar())
			appended = std::string(lhs.GetString()) + std::string(1, (char)rhs.m_Value);
		else if (lhs.IsString() && rhs.IsChar())
			appended = std::string(lhs.GetString()) + std::string(1, (char)rhs.m_Value);*/

			// Allocate a new object in the heap that stores the string
		//HeapEntry& newObject = BytecodeInterpreter::Get().m_Heap.CreateString(appended);
		return Value(appended, ValueTypes::String);
	}

	/*if (lhs.IsArray())
	{
		ValueArray arr;
		arr.push_back(lhs);
		arr.push_back(rhs);
		return Functions::array_concat(&arr);
	}*/

	return MakeRuntimeError("Unhandled addition of types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));
}

Value Value::Subtract(Value& lhs, Value& rhs)
{
	if (!Value::IsSamePrimitiveType(lhs, rhs))
		return MakeRuntimeError("Cannot subtract types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));

	if (lhs.m_Type == ValueTypes::Integer)
		return Value(lhs.GetInt() - rhs.GetInt(), ValueTypes::Integer);
	else if (lhs.m_Type == ValueTypes::Float)
		return Value(lhs.GetFloat() - rhs.GetFloat(), ValueTypes::Float);
	else if (lhs.IsString())
		return MakeRuntimeError("Cannot subtract two strings");

	return MakeRuntimeError("Unhandled subtraction of types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));
}

Value Value::Divide(Value& lhs, Value& rhs)
{
	if (!Value::IsSamePrimitiveType(lhs, rhs))
		return MakeRuntimeError("Cannot divide types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));

	if (lhs.m_Type == ValueTypes::Integer)
	{
		if (rhs.GetInt() == 0) return MakeRuntimeError("Division by 0");

		return Value(lhs.GetInt() / rhs.GetInt(), ValueTypes::Integer);
	}
	else if (lhs.m_Type == ValueTypes::Float)
	{
		if (rhs.GetFloat() == 0) return MakeRuntimeError("Division by 0");

		return Value(lhs.GetFloat() / rhs.GetFloat(), ValueTypes::Float);
	}

	else if (lhs.IsString())
		return MakeRuntimeError("Cannot divide two strings");

	return MakeRuntimeError("Unhandled division of types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));
}

Value Value::Multiply(Value& lhs, Value& rhs)
{
	if (!Value::IsSamePrimitiveType(lhs, rhs))
		return MakeRuntimeError("Cannot multiply types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));

	if (lhs.m_Type == ValueTypes::Integer)
		return Value(lhs.GetInt() * rhs.GetInt(), ValueTypes::Integer);
	else if (lhs.m_Type == ValueTypes::Float)
		return Value(lhs.GetFloat() * rhs.GetFloat(), ValueTypes::Float);
	else if (lhs.IsString())
		return MakeRuntimeError("Cannot multiply two strings");

	return MakeRuntimeError("Unhandled multiplication of types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));
}

Value::~Value()
{

}

	//Value Value::Pow(Value& base, Value& exponent)
	//{
	//	if (!Value::IsSamePrimitiveType(base, exponent))
	//		return MakeRuntimeError("Cannot do exponents with types " + ValueTypeToString(base.m_Type) + " and " + ValueTypeToString(exponent.m_Type));
	//
	//	if (base.m_Type == Value::Number)
	//	{
	//		// Check for imaginary numbers. base < 0 and exponent is not a whole number
	//		if (base.m_Value < 0 && (std::floor(exponent.m_Value) != exponent.m_Value))
	//			return MakeRuntimeError("Undefined math operation. Imaginary number");
	//
	//		return Value(pow(base.m_Value, exponent.m_Value), Value::Number);
	//	}
	//	else if (base.IsString())
	//		return MakeRuntimeError("Cannot pow two strings");
	//
	//	return MakeRuntimeError("Unhandled pow of types " + ValueTypeToString(base.m_Type) + " and " + ValueTypeToString(exponent.m_Type));
	//}
	//
	//Value Value::Mod(Value& lhs, Value& rhs)
	//{
	//	if (!Value::IsSamePrimitiveType(lhs, rhs))
	//		return MakeRuntimeError("Cannot mod types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));
	//
	//	if (lhs.m_Type == Value::Number)
	//	{
	//		if (rhs.m_Value == 0)
	//			return MakeRuntimeError("Undefined math operation. Cannot mod something with 0");
	//
	//		return Value((int)lhs.m_Value % (int)rhs.m_Value, Value::Number);
	//	}
	//
	//	return MakeRuntimeError("Cannot perform modulus on types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));
	//}
	//
	//Value Value::Xor(Value& lhs, Value& rhs)
	//{
	//	if (!Value::IsSamePrimitiveType(lhs, rhs))
	//		return MakeRuntimeError("Cannot xor types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));
	//
	//	if (lhs.m_Type == Value::Number)
	//		return Value((int)lhs.m_Value ^ (int)rhs.m_Value, Value::Number);
	//
	//	return MakeRuntimeError("Cannot perform xor on types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));
	//}

	//HeapEntry& Heap::CreateObject(int type, char* data)
	//{
	//	int id = m_NextFreeId;
	//	m_Entries[id] = HeapEntry(0, id, data);

	//	m_NextFreeId++; // Increment to use a different slot for the next object

	//	return m_Entries[id];
	//}

	//HeapEntry& Heap::CreateString(const std::string& str)
	//{
	//	int id = m_NextFreeId;
	//	m_Entries[id] = HeapEntry(0, id, (const char*)nullptr);

	//	char* strData = CopyString(str.c_str());

	//	m_Entries[id].m_Data = (void*)strData;

	//	m_NextFreeId++; // Increment to use a different slot for the next object

	//	return m_Entries[id];
	//}

	//HeapEntry& Heap::CreateArray()
	//{
	//	int id = m_NextFreeId;
	//	m_Entries[id] = HeapEntry(1, id, (const char*)nullptr);

	//	ValueArray* arrayData = new ValueArray();
	//	m_Entries[id].m_Data = (void*)arrayData;

	//	m_NextFreeId++; // Increment to use a different slot for the next object

	//	return m_Entries[id];
	//}

	//HeapEntry& Heap::CreateObject()
	//{
	//	int id = m_NextFreeId;
	//	m_Entries[id] = HeapEntry(2, id, (const char*)nullptr);

	//	ObjectInstance* objectData = new ObjectInstance();
	//	m_Entries[id].m_Data = (void*)objectData;

	//	m_NextFreeId++; // Increment to use a different slot for the next object

	//	return m_Entries[id];
	//}

	//void Heap::DeleteObject(HeapEntry* obj)
	//{
	//	m_Entries.erase(obj->m_Id);

	//	obj->m_Data = nullptr;
	//}

	//void Heap::DeleteObject(int id)
	//{
	//	delete m_Entries[id].m_Data;
	//	m_Entries[id].m_Data = nullptr;

	//	m_Entries.erase(id);
	//}

	//std::string HeapEntry::ToString()
	//{
	//	return Value(*this).ToFormattedString(true);
	//}
