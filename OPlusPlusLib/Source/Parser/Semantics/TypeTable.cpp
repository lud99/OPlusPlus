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

	TypeTableEntry& TypeTable::Add(const std::string& typeName, TypeTableType type, TypeTableEntry* redirect)
	{
		assert (!HasType(typeName));

		uint16_t id = m_Types.size();

		m_TypeNames[typeName] = id;
		m_Types.push_back({ typeName, id, type, redirect });

		return m_Types[id];
	}
	TypeTableEntry& TypeTable::AddGeneric(TypeTableType type, std::vector<TypeTableEntry> typeArguments)
	{
		assert(!typeArguments.empty());
		if (type == TypeTableType::Array)
			assert(typeArguments.size() == 1);
		// TODO: Validate the rest of generic types

		std::string name = TypeTableTypeToString(type) + "<";
		for (int i = 0; i < typeArguments.size() - 1; i++)
		{
			name += typeArguments[i].name + ", ";
		}
		name += typeArguments.back().name + ">";

		if (HasType(name))
			return GetType(name);

		TypeTableEntry& typeEntry = Add(name, type);

		// Set the type arguments
		for (auto& argument : typeArguments)
		{
			typeEntry.typeArguments.push_back(argument.id);
		}

		return typeEntry;
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
	TypeTableEntry& TypeTable::AddArray(TypeTableEntry& underlyingType)
	{
		return AddGeneric(TypeTableType::Array, { underlyingType });
	}
	TypeTableEntry& TypeTable::AddTuple(std::vector<TypeTableEntry> underlyingTypes)
	{
		return AddGeneric(TypeTableType::Tuple, underlyingTypes);
	}
	TypeTableEntry& TypeTable::AddFunction(std::vector<TypeTableEntry> argumentTypes, TypeTableEntry returnType)
	{
		std::vector<TypeTableEntry> typeArguments = argumentTypes;
		typeArguments.push_back(returnType);

		return AddGeneric(TypeTableType::Function, typeArguments);
	}

	void TypeTable::AddTypeRelation(TypeTableEntry& type, ValueType relatedType, TypeRelation::ConversionType subtypeConversion, TypeRelation::ConversionType supertypeConversion)
	{
		AddTypeRelation(type, GetType(relatedType), subtypeConversion, supertypeConversion);
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