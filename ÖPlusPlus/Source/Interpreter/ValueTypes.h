#pragma once

enum ValueTypes
{
	Void,
	Integer,
	Float,
	String,
	StringReference,
	StringConstant,
};

static std::string ValueTypeToString(ValueTypes type)
{
	std::string names[] = { "Empty", "Integer", "Float", "String", "StringReference", "StringConstant" };
	return names[(int)type];
}