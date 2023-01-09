#pragma once

enum ValueTypes
{
	Void,
	Integer,
	Float,
	String,
	StringReference,
	StringConstant,
	Any,
};

static std::string ValueTypeToString(ValueTypes type)
{
	std::string names[] = { "Empty", "Integer", "Float", "String", "StringReference", "StringConstant", "Any" };
	return names[(int)type];
}