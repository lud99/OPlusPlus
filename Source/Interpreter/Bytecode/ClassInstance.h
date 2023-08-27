#pragma once

#include "Instruction.h"
#include "../Value.h"

namespace Ö::Bytecode
{
	class ClassInstance
	{
	public:
		ClassInstance() {};
		ClassInstance(std::string name, uint16_t index) : m_Name(name), m_Index(index) {};

	public:
		std::string m_Name;
		uint16_t m_Index = 0;

		Instructions m_InternalConstructor;
		EncodedInstructions m_InternalConstructorEncoded;
		std::unordered_map<uint16_t, Instructions> m_Methods;
		std::unordered_map<uint16_t, EncodedInstructions> m_MethodsEncoded;

		std::unordered_map<uint16_t, Value> m_MemberVariables;
	};
}