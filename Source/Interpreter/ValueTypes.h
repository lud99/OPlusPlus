#pragma once

#include <cstdint>
#include <string>

namespace Ö
{
	enum ValueTypes
	{
		Void = 0,
		Integer = 1,
		Float = 2,
		String = 3,
		/*StringReference,
		StringConstant,

		Any,*/
	};

	typedef uint16_t ValueType;

	static std::string ValueTypeToString(ValueTypes type)
	{
		std::string names[] = { "Void", "Integer", "Float", "String", /*"StringReference", "StringConstant", "Void", "Any"*/ };
		return names[(int)type];
	}
}
