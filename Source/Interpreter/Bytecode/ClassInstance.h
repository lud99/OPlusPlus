#pragma once

#include "Instruction.h"
#include "../Value.h"

namespace Ö::Bytecode
{
	struct CompiledCallable
	{
		ValueType returnType;
		std::vector<ValueType> parameters;
		uint16_t constantsPoolIndex;

		Instructions body;
		EncodedInstructions encodedBody;
	};

	class ClassInstance
	{
	public:
		ClassInstance() {};
		ClassInstance(std::string name, uint16_t index) : m_Name(name), m_Index(index) {};

		struct MemberVariable
		{
			ValueType type;
			Value value;
		};

	public:
		std::string m_Name;
		uint16_t m_Index = 0;

		Instructions m_InternalConstructor;
		EncodedInstructions m_InternalConstructorEncoded;

		std::unordered_map<uint16_t, CompiledCallable> m_Methods;
		std::unordered_map<uint16_t, MemberVariable> m_MemberVariables;
	};
}