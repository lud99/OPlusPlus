#include "ConstantsPool.h"

namespace Ö
{
	ConstantPoolType RuntimeConstantsPool::GetTypeOfConstant(uint16_t index)
	{
		if (m_Integers.count(index) == 1)
			return ConstantPoolType::Integer;
		if (m_Floats.count(index) == 1)
			return ConstantPoolType::Float;
		if (m_Strings.count(index) == 1)
			return ConstantPoolType::String;

		abort();
		return ConstantPoolType::Integer;
	}

	int32_t RuntimeConstantsPool::GetInteger(uint16_t index)
	{
		// TODO: ensure it exist might not be necessary
		assert(m_Integers.count(index) == 1);
		return m_Integers[index];
	}

	float RuntimeConstantsPool::GetFloat(uint16_t index)
	{
		assert(m_Floats.count(index) == 1);
		return m_Floats[index];
	}

	const std::string& RuntimeConstantsPool::GetString(uint16_t index)
	{
		assert(m_Strings.count(index) == 1);
		return m_Strings[index];
	}
}