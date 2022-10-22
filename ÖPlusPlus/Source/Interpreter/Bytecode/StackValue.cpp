#include "StackValue.h"

#include "BytecodeInterpreter.h"

#include <iostream>

#include "../../Utils.hpp"

char* StackValue::GetString()
{
	assert(IsString());

	if (_m_String != "") return &_m_String[0];

	return (char*)(m_HeapEntryPointer)->m_Data;
}

int StackValue::GetInt()
{
	assert(m_Type == ValueTypes::Integer);

	return m_ValueInt;
}

double StackValue::GetFloat()
{
	assert(m_Type == ValueTypes::Float);

	return m_ValueFloat;
}

/*StackValueArray* StackValue::GetArray()
{
	assert(IsArray());

	return (StackValueArray*)(m_HeapEntryPointer)->m_Data;
}

ObjectInstance* StackValue::GetObjectInstance()
{
	assert(m_Type == Value::Object);

	return (ObjectInstance*)(m_HeapEntryPointer)->m_Data;
}*/

std::string StackValue::ToString()
{
	return ToFormattedString(false);
}

std::string StackValue::ToFormattedString(bool extraQuotes, int currentDepth)
{
	if (m_Type == ValueTypes::Void)
		return "(empty)";

	if (m_Type == ValueTypes::Integer)
	{
		return std::to_string(m_ValueInt);
	}
	if (m_Type == ValueTypes::Float)
	{
		return std::to_string(m_ValueFloat);
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
		StackValueArray* arr = GetArray();
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

StackValue::StackValue(HeapEntry& value)
{
	m_HeapEntryPointer = &value;

	if (value.m_Type == 0)
		m_Type = ValueTypes::StringReference;
	/*else if (value.m_Type == 1)
		m_Type = Value::Array;
	else if (value.m_Type == 2)
		m_Type = Value::Object;*/
}

bool StackValue::IsTruthy()
{
	assert (m_Type != ValueTypes::Void);

	if (IsString())
		return std::string(GetString()) != "";

	if (m_Type == ValueTypes::Integer)
		return GetInt() > 0;
	if (m_Type == ValueTypes::Float)
		return GetFloat() > 0;

	/*if (m_Type == Value::Array)
		return !GetArray()->empty();

	if (m_Type == Value::Object)
		return GetObjectInstance()->size() != 0;*/

	abort();
	return false;
}

bool StackValue::IsSamePrimitiveType(StackValue lhs, StackValue rhs)
{
	if (lhs.IsString() && rhs.IsString())
		return true;

	//if ((lhs.IsString() && rhs.IsChar()) || (lhs.IsChar() && rhs.IsString()));
	//return true;

	if (lhs.m_Type == rhs.m_Type)
		return true;

	/*if ((lhs.m_Type == Value::Function && rhs.m_Type == Value::Number) ||
		(lhs.m_Type == Value::Number && rhs.m_Type == Value::Function))
		return true;*/

	return false;
}

bool StackValue::IsSamePrimitiveType(ValueTypes lhs, ValueTypes rhs)
{
	return IsSamePrimitiveType(StackValue(lhs), StackValue(rhs));
}

bool StackValue::Compare(StackValue lhs, StackValue rhs, ASTTypes comparisonType)
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

void StackValue::Delete()
{
	if (IsString())
	{
		// Delete the string, because strings aren't stored as a reference to each other. Don't delete constants!!!
		if (m_Type == ValueTypes::StringReference)
			BytecodeInterpreter::Get().m_Heap.DeleteObject(m_HeapEntryPointer);
	}

	m_ValueInt = 0;
	m_Type = ValueTypes::Void;
}

bool StackValue::CompareEquals(StackValue lhs, StackValue rhs)
{
	if (!StackValue::IsSamePrimitiveType(lhs, rhs)) return false;

	ValueTypes& type = lhs.m_Type;

	// If both left and right are null
	//if (type == Value::Null)
		//return true;

	// Numeric
	if (type == ValueTypes::Integer)
		return lhs.GetInt() == rhs.GetInt();
	if (type == ValueTypes::Float)
		return lhs.GetFloat() == rhs.GetFloat();

	//// Char and char
	//if (lhs.IsChar() && rhs.IsChar())
	//	return (int)lhs.m_Value == (int)rhs.m_Value;

	// String and char
	/*if (lhs.IsChar() && rhs.IsString())
		return std::string(1, (char)lhs.m_Value) == std::string(rhs.GetString());
	if (lhs.IsString() && rhs.IsChar())
		return std::string(lhs.GetString()) == std::string(1, (char)rhs.m_Value);*/

	// String reference
	if (lhs.IsString())
		return std::string(lhs.GetString()) == std::string(rhs.GetString());

	abort();
	return false;
}

bool StackValue::CompareNotEquals(StackValue lhs, StackValue rhs)
{
	if (!StackValue::IsSamePrimitiveType(lhs, rhs)) return true;

	ValueTypes& type = lhs.m_Type;

	// If both left and right are null
	//if (type == Value::Null)
	//	return false;

	//// Numeric
	//if (type == Value::Number || type == Value::Bool)
	//	return lhs.m_Value != rhs.m_Value;

	// Numeric
	if (type == ValueTypes::Integer)
		return lhs.GetInt() != rhs.GetInt();
	if (type == ValueTypes::Float)
		return lhs.GetFloat() != rhs.GetFloat();

	// String reference
	if (lhs.IsString())
		return std::string(lhs.GetString()) != std::string(rhs.GetString());

	abort();

	return false;
}

bool StackValue::CompareLessThan(StackValue lhs, StackValue rhs)
{
	if (!StackValue::IsSamePrimitiveType(lhs, rhs)) return false;

	ValueTypes& type = lhs.m_Type;

	// If both left and right are null
	//if (type == Value::Null)
	//	return false;

	//// Numeric
	//if (type == Value::Number || type == Value::Bool)
	//	return lhs.m_Value < rhs.m_Value;

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

bool StackValue::CompareGreaterThan(StackValue lhs, StackValue rhs)
{
	if (!StackValue::IsSamePrimitiveType(lhs, rhs)) return false;

	ValueTypes& type = lhs.m_Type;

	// If both left and right are null
	/*if (type == Value::Null)
		return false;*/

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

bool StackValue::CompareLessThanEqual(StackValue lhs, StackValue rhs)
{
	if (!StackValue::IsSamePrimitiveType(lhs, rhs)) return false;

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

bool StackValue::CompareGreaterThanEqual(StackValue lhs, StackValue rhs)
{
	if (!StackValue::IsSamePrimitiveType(lhs, rhs)) return false;

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

StackValue StackValue::Add(StackValue& lhs, StackValue& rhs)
{
	if (!StackValue::IsSamePrimitiveType(lhs, rhs))
		return BytecodeInterpreter::Get().ThrowExceptionValue("Cannot add types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));

	if (lhs.m_Type == ValueTypes::Integer)
	{
		return StackValue(lhs.GetInt() + rhs.GetInt(), ValueTypes::Integer);
	}
	else if (lhs.m_Type == ValueTypes::Float)
	{
		return StackValue(lhs.GetFloat() + rhs.GetFloat(), ValueTypes::Float);
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
		HeapEntry& newObject = BytecodeInterpreter::Get().m_Heap.CreateString(appended);
		return StackValue(newObject);
	}

	/*if (lhs.IsArray())
	{
		StackValueArray arr;
		arr.push_back(lhs);
		arr.push_back(rhs);
		return Functions::array_concat(&arr);
	}*/

	return BytecodeInterpreter::Get().ThrowExceptionValue("Unhandled addition of types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));
}

StackValue StackValue::Subtract(StackValue& lhs, StackValue& rhs)
{
	if (!StackValue::IsSamePrimitiveType(lhs, rhs))
		return BytecodeInterpreter::Get().ThrowExceptionValue("Cannot subtract types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));

	if (lhs.m_Type == ValueTypes::Integer)
		return StackValue(lhs.GetInt() - rhs.GetInt(), ValueTypes::Integer);
	else if (lhs.m_Type == ValueTypes::Float)
		return StackValue(lhs.GetFloat() - rhs.GetFloat(), ValueTypes::Float);
	else if (lhs.IsString())
		return BytecodeInterpreter::Get().ThrowExceptionValue("Cannot subtract two strings");

	return BytecodeInterpreter::Get().ThrowExceptionValue("Unhandled subtraction of types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));
}

StackValue StackValue::Divide(StackValue& lhs, StackValue& rhs)
{
	if (!StackValue::IsSamePrimitiveType(lhs, rhs))
		return BytecodeInterpreter::Get().ThrowExceptionValue("Cannot divide types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));

	if (lhs.m_Type == ValueTypes::Integer)
	{
		if (rhs.GetInt() == 0) return BytecodeInterpreter::Get().ThrowExceptionValue("Division by 0");

		return StackValue(lhs.GetInt() / rhs.GetInt(), ValueTypes::Integer);
	} else if (lhs.m_Type == ValueTypes::Float)
	{
		if (rhs.GetFloat() == 0) return BytecodeInterpreter::Get().ThrowExceptionValue("Division by 0");

		return StackValue(lhs.GetFloat() / rhs.GetFloat(), ValueTypes::Float);
	}

	else if (lhs.IsString())
		return BytecodeInterpreter::Get().ThrowExceptionValue("Cannot divide two strings");

	return BytecodeInterpreter::Get().ThrowExceptionValue("Unhandled division of types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));
}

StackValue StackValue::Multiply(StackValue& lhs, StackValue& rhs)
{
	if (!StackValue::IsSamePrimitiveType(lhs, rhs))
		return BytecodeInterpreter::Get().ThrowExceptionValue("Cannot multiply types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));

	if (lhs.m_Type == ValueTypes::Integer)
		return StackValue(lhs.GetInt() * rhs.GetInt(), ValueTypes::Integer);
	else if (lhs.m_Type == ValueTypes::Float)
		return StackValue(lhs.GetFloat() * rhs.GetFloat(), ValueTypes::Float);
	else if (lhs.IsString())
		return BytecodeInterpreter::Get().ThrowExceptionValue("Cannot multiply two strings");

	return BytecodeInterpreter::Get().ThrowExceptionValue("Unhandled multiplication of types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));
}

//StackValue StackValue::Pow(StackValue& base, StackValue& exponent)
//{
//	if (!StackValue::IsSamePrimitiveType(base, exponent))
//		return BytecodeInterpreter::Get().ThrowExceptionValue("Cannot do exponents with types " + ValueTypeToString(base.m_Type) + " and " + ValueTypeToString(exponent.m_Type));
//
//	if (base.m_Type == Value::Number)
//	{
//		// Check for imaginary numbers. base < 0 and exponent is not a whole number
//		if (base.m_Value < 0 && (std::floor(exponent.m_Value) != exponent.m_Value))
//			return BytecodeInterpreter::Get().ThrowExceptionValue("Undefined math operation. Imaginary number");
//
//		return StackValue(pow(base.m_Value, exponent.m_Value), Value::Number);
//	}
//	else if (base.IsString())
//		return BytecodeInterpreter::Get().ThrowExceptionValue("Cannot pow two strings");
//
//	return BytecodeInterpreter::Get().ThrowExceptionValue("Unhandled pow of types " + ValueTypeToString(base.m_Type) + " and " + ValueTypeToString(exponent.m_Type));
//}
//
//StackValue StackValue::Mod(StackValue& lhs, StackValue& rhs)
//{
//	if (!StackValue::IsSamePrimitiveType(lhs, rhs))
//		return BytecodeInterpreter::Get().ThrowExceptionValue("Cannot mod types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));
//
//	if (lhs.m_Type == Value::Number)
//	{
//		if (rhs.m_Value == 0)
//			return BytecodeInterpreter::Get().ThrowExceptionValue("Undefined math operation. Cannot mod something with 0");
//
//		return StackValue((int)lhs.m_Value % (int)rhs.m_Value, Value::Number);
//	}
//
//	return BytecodeInterpreter::Get().ThrowExceptionValue("Cannot perform modulus on types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));
//}
//
//StackValue StackValue::Xor(StackValue& lhs, StackValue& rhs)
//{
//	if (!StackValue::IsSamePrimitiveType(lhs, rhs))
//		return BytecodeInterpreter::Get().ThrowExceptionValue("Cannot xor types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));
//
//	if (lhs.m_Type == Value::Number)
//		return StackValue((int)lhs.m_Value ^ (int)rhs.m_Value, Value::Number);
//
//	return BytecodeInterpreter::Get().ThrowExceptionValue("Cannot perform xor on types " + ValueTypeToString(lhs.m_Type) + " and " + ValueTypeToString(rhs.m_Type));
//}

HeapEntry& Heap::CreateObject(int type, char* data)
{
	int id = m_NextFreeId;
	m_Entries[id] = HeapEntry(0, id, data);

	m_NextFreeId++; // Increment to use a different slot for the next object

	return m_Entries[id];
}

HeapEntry& Heap::CreateString(const std::string& str)
{
	int id = m_NextFreeId;
	m_Entries[id] = HeapEntry(0, id, (const char*)nullptr);

	char* strData = CopyString(str.c_str());

	m_Entries[id].m_Data = (void*)strData;

	m_NextFreeId++; // Increment to use a different slot for the next object

	return m_Entries[id];
}

HeapEntry& Heap::CreateArray()
{
	int id = m_NextFreeId;
	m_Entries[id] = HeapEntry(1, id, (const char*)nullptr);

	StackValueArray* arrayData = new StackValueArray();
	m_Entries[id].m_Data = (void*)arrayData;

	m_NextFreeId++; // Increment to use a different slot for the next object

	return m_Entries[id];
}

HeapEntry& Heap::CreateObject()
{
	int id = m_NextFreeId;
	m_Entries[id] = HeapEntry(2, id, (const char*)nullptr);

	ObjectInstance* objectData = new ObjectInstance();
	m_Entries[id].m_Data = (void*)objectData;

	m_NextFreeId++; // Increment to use a different slot for the next object

	return m_Entries[id];
}

void Heap::DeleteObject(HeapEntry* obj)
{
	m_Entries.erase(obj->m_Id);

	obj->m_Data = nullptr;
}

void Heap::DeleteObject(int id)
{
	delete m_Entries[id].m_Data;
	m_Entries[id].m_Data = nullptr;

	m_Entries.erase(id);
}

std::string HeapEntry::ToString()
{
	return StackValue(*this).ToFormattedString(true);
}
