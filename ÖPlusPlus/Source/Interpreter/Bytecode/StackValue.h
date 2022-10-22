#pragma once

#include "../../Parser.h"

#include <string>

#include <map>
#include <unordered_map>
#include <vector>

class HeapEntry
{
public:
	HeapEntry() {};
	HeapEntry(int type, int id, char* data = nullptr) : m_Type(type), m_Id(id), m_Data(data) {};
	HeapEntry(int type, int id, const char* data = nullptr) : m_Type(type), m_Id(id), m_Data((char*)data) {};

	std::string ToString();

public:
	int m_Type = 0;
	int m_Id = 0;

	void* m_Data = nullptr;
};

constexpr int STACK_SIZE = 256;
constexpr int STACKFRAME_SIZE = 32;

enum ValueTypes
{
	Void,
	Integer,
	Float,
	String,
	StringReference,
	StringConstant,
};

class StackValue
{
public:
	ValueTypes m_Type = ValueTypes::Void;

	double m_ValueFloat = 0.0;
	int m_ValueInt = 0;

	std::string _m_String = "";

	HeapEntry* m_HeapEntryPointer = nullptr;
	
	StackValue() {};
	StackValue(ValueTypes type) : m_Type(type) {};
	StackValue(int value, ValueTypes type) : m_ValueInt(value), m_Type(type) {};
	StackValue(double value, ValueTypes type) : m_ValueFloat(value), m_Type(type) {};
	StackValue(HeapEntry& value);
	StackValue(HeapEntry& value, ValueTypes type) : m_HeapEntryPointer(&value), m_Type(type) {};

	inline bool IsString() { return m_Type == ValueTypes::String || m_Type == ValueTypes::StringReference || m_Type == ValueTypes::StringConstant; };
	inline bool IsObject() { return IsString(); };
	bool IsTruthy();

	char* GetString();
	int GetInt();
	double GetFloat();
	/*std::vector<StackValue>* GetArray();
	std::map<std::string, StackValue>* GetObjectInstance();*/

	// Same as ToFormattedString, but without extra quotes for use internally
	std::string ToString();
	std::string ToFormattedString(bool extraQuotes = true, int currentDepth = 0);

	void Delete();

	static bool IsSamePrimitiveType(StackValue lhs, StackValue rhs);
	static bool IsSamePrimitiveType(ValueTypes lhs, ValueTypes rhs);

	static bool Compare(StackValue lhs, StackValue rhs, ASTTypes comparisonType);
	static bool CompareEquals(StackValue lhs, StackValue rhs);
	static bool CompareNotEquals(StackValue lhs, StackValue rhs);
	static bool CompareLessThan(StackValue lhs, StackValue rhs);
	static bool CompareGreaterThan(StackValue lhs, StackValue rhs);
	static bool CompareLessThanEqual(StackValue lhs, StackValue rhs);
	static bool CompareGreaterThanEqual(StackValue lhs, StackValue rhs);

	static StackValue Add(StackValue& lhs, StackValue& rhs);
	static StackValue Subtract(StackValue& lhs, StackValue& rhs);
	static StackValue Divide(StackValue& lhs, StackValue& rhs);
	static StackValue Multiply(StackValue& lhs, StackValue& rhs);
	/*static StackValue Pow(StackValue& base, StackValue& exponent);
	static StackValue Mod(StackValue& lhs, StackValue& rhs);
	static StackValue Xor(StackValue& lhs, StackValue& rhs);*/

	~StackValue() {};
};

static std::string ValueTypeToString(ValueTypes type)
{
	std::string names[] = { "Empty", "Integer", "Float", "String", "StringReference", "StringConstant" };
	return names[(int)type];
}

typedef std::vector<StackValue> StackValueArray;
typedef std::map<std::string, StackValue> ObjectInstance;

struct Heap
{
	std::unordered_map<int, HeapEntry> m_Entries;

	int m_NextFreeId = 0;

	HeapEntry& CreateObject(int type, char* data = nullptr);

	HeapEntry& CreateString(const std::string& str = "");
	HeapEntry& CreateArray();
	HeapEntry& CreateObject();

	void DeleteObject(HeapEntry* obj);
	void DeleteObject(int id);
};
