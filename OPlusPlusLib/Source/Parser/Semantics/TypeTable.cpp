#include "TypeTable.h"

#include <assert.h>

namespace O
{
	TypeTableEntry& TypeTableEntry::Resolve()
	{
		TypeTableEntry* currentEntry = this;
		while (currentEntry->type == TypeTableType::Typedef)
		{
			assert(currentEntry->redirect != nullptr);

			currentEntry = currentEntry->redirect;
		}

		return *currentEntry;
	}

	TypeTable::TypeTable()
	{
		// Add the build in primitives to the typeEntry table 
		std::vector<std::string> typeKeywords = { "void", "int", "bool", "float", "string" };

		for (uint16_t i = 0; i < typeKeywords.size(); i++)
		{
			m_TypeNames[typeKeywords[i]] = i;
			m_Types.push_back({ typeKeywords[i], i, TypeTableType::Primitive });
		}

		// Add relation for types
		AddTypeRelation(m_Types[PrimitiveValueTypes::Float], PrimitiveValueTypes::Integer, TypeRelation::Implicit, TypeRelation::Explicit);
		AddTypeRelation(m_Types[PrimitiveValueTypes::Integer], PrimitiveValueTypes::Bool, TypeRelation::Implicit, TypeRelation::Explicit);
	}

	TypeTableEntry& TypeTable::GetEntryFromId(ValueType id)
	{
		return m_Types[id];
	}

	TypeTableEntry& TypeTable::Add(const std::string& typeName, TypeTableType type, TypeTableEntry* redirect)
	{
		assert (!HasType(typeName));

		uint16_t id = m_Types.size();

		m_TypeNames[typeName] = id;
		m_Types.push_back({ typeName, id, type, redirect });

		return m_Types[id];
	}
	TypeTableEntry& TypeTable::AddPrivateType(const std::string& typeName, TypeTableType type, TypeTableEntry* redirect)
	{
		assert (!HasType(typeName));

		uint16_t id = m_Types.size();

		m_TypeNames[typeName] = id;
		m_Types.push_back({ typeName, id, type, redirect });
		m_Types[id].isPrivate = true;

		return m_Types[id];
	}
	ValueType TypeTable::AddArray(ValueType underlyingType)
	{
		TypeTableEntry& type = Add("", TypeTableType::Array);
		type.underlyingType = underlyingType;

		return type.id;
	}
	void TypeTable::AddTypeRelation(TypeTableEntry& type, ValueType relatedType, TypeRelation::ConversionType subtypeConversion, TypeRelation::ConversionType supertypeConversion)
	{
		AddTypeRelation(type, GetEntryFromId(relatedType), subtypeConversion, supertypeConversion);
	}
	void TypeTable::AddTypeRelation(TypeTableEntry& type, TypeTableEntry& relatedType, TypeRelation::ConversionType subtypeConversion, TypeRelation::ConversionType supertypeConversion)
	{
		type.subtypes.push_back({ subtypeConversion, relatedType.id });
		relatedType.supertypes.push_back({ supertypeConversion, type.id });
	}
	TypeTableEntry& TypeTable::ResolveEntry(TypeTableEntry entry)
	{
		return entry.Resolve();
	}
}