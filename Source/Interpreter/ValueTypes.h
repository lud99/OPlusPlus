#pragma once

#include <cstdint>
#include <string>
#include <assert.h>

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

	/*bool IsValueTypePrimitive(ValueType type)
	{
		assert(type != 0);
		return type == 1 || type == 2 || type == 3;
	}*/

	static std::string ValueTypeToString(ValueTypes type)
	{
		std::string names[] = { "Void", "Integer", "Float", "String", /*"StringReference", "StringConstant", "Void", "Any"*/ };
		return names[(int)type];
	}
}
