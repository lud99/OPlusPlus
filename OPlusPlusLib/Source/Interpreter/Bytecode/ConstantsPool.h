#pragma once

#include <unordered_map>
#include <string>

#include "../Functions.h"

namespace Ö
{

	enum class ConstantPoolType
	{
		Integer,
		Float,
		Double,
		Long,
		String,
		FunctionReference,
		MethodReference,
		ClassReference,
		InnerClassReference
	};

	class RuntimeConstantsPool
	{
	public:

		//bool HasConstant(ConstantPoolType typeEntry, uint16_t index);

		ConstantPoolType GetTypeOfConstant(uint16_t index);

		int32_t GetInteger(uint16_t index);
		float GetFloat(uint16_t index);
		const std::string& GetString(uint16_t index);
		//uint16_t AddOrGetFunctionReferenceIndex(std::string functionName);

	public:
		std::unordered_map<uint16_t, int32_t> m_Integers;
		std::unordered_map<uint16_t, float> m_Floats;
		std::unordered_map<uint16_t, std::string> m_Strings;

		std::unordered_map<uint16_t, std::string> m_FunctionReferences;
		std::unordered_map<uint16_t, std::string> m_MethodReferences;
		std::unordered_map<uint16_t, std::string> m_ClassReferences;

		std::unordered_map<uint16_t, BuiltInFunctions::Prototype> m_BuiltInFunctions;

	};

}

// > 16 bit are stored in constants pool
// floats are always stored, indexes are uint8
// When index is larger, use uint16
// signed byte push
// signed short push