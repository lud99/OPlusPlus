#include "TypeTable.h"

#include <assert.h>
#include <iostream>

namespace O
{
	TypeTableEntry& TypeTableEntry::Resolve()
	{
		TypeTableEntry* currentEntry = this;
		while (currentEntry->type == TypeEntryType::Typedef)
		{
			assert(currentEntry->redirect != nullptr);

			currentEntry = currentEntry->redirect;
		}

		return *currentEntry;
	}

	TypeTable::TypeTable()
	{
		InsertPrimitiveTypes();
		m_TableType = TypeTableType::Global;
	}

	TypeTable::TypeTable(TypeTableType tableType, TypeTable* upwardTypeTable)
	{
		if (tableType == TypeTableType::Global)
			InsertPrimitiveTypes();

		m_TableType = tableType;
		m_UpwardTypeTable = upwardTypeTable;
	}

	bool TypeTable::Has(const std::string& typeName)
	{
		// First look in the current table
		if (m_Typenames.count(typeName) != 0)
			return true;

		// Otherwise look upward
		if (m_UpwardTypeTable)
			return m_UpwardTypeTable->Has(typeName);

		assert(m_TableType == TypeTableType::Global);
		return false;
	}

	bool TypeTable::Has(ValueType typeId)
	{
		// First look in the current table
		for (auto& type : m_Types)
		{
			if (type.id == typeId)
				return true;
		}

		// Otherwise look upward
		if (m_UpwardTypeTable)
			return m_UpwardTypeTable->Has(typeId);

		assert(m_TableType == TypeTableType::Global);
		return false;
	}

	TypeTableEntry* TypeTable::Lookup(const std::string& typeName)
	{
		// First look in the current table
		if (m_Typenames.count(typeName) != 0)
			return &m_Types[m_Typenames[typeName]]; // TODO: Unify typenames and types

		// Otherwise look upward
		if (m_UpwardTypeTable)
			return m_UpwardTypeTable->Lookup(typeName);

		assert(m_TableType == TypeTableType::Global);
	}

	TypeTableEntry* TypeTable::Lookup(ValueType typeId)
	{
		// First look in the current table
		for (auto& type : m_Types)
		{
			if (type.id == typeId)
				return &type;
		}

		// Otherwise look upward
		if (m_UpwardTypeTable)
			return m_UpwardTypeTable->Lookup(typeId);

		assert(m_TableType == TypeTableType::Global);
		return nullptr;
	}

	TypeTableEntry& TypeTable::Insert(const std::string& typeName, TypeEntryType type, TypeTableEntry* redirect)
	{
		assert(!Has(typeName));

		uint16_t id = m_Types.size();

		m_Typenames[typeName] = id;
		m_Types.push_back({ typeName, id, type, redirect });

		return m_Types[id];
	}
	TypeTableEntry& TypeTable::InsertGeneric(TypeEntryType type, std::vector<TypeTableEntry> typeArguments)
	{
		assert(!typeArguments.empty());
		if (type == TypeEntryType::Array)
			assert(typeArguments.size() == 1);
		// TODO: Validate the rest of generic types

		std::string name = TypeEntryTypeToString(type) + "<";
		for (int i = 0; i < typeArguments.size() - 1; i++)
		{
			name += typeArguments[i].name + ", ";
		}
		name += typeArguments.back().name + ">";

		if (Has(name))
			return *Lookup(name);

		TypeTableEntry& typeEntry = Insert(name, type);

		// Set the type arguments
		for (auto& argument : typeArguments)
		{
			typeEntry.typeArguments.push_back(argument.id);
		}

		return typeEntry;
	}
	/*TypeTableEntry& TypeTable::InsertPrivateType(const std::string& typeName, TypeEntryType type, TypeTableEntry* redirect)
	{
		assert (!Has(typeName));

		uint16_t id = m_Types.size();

		m_Typenames[typeName] = id;
		m_Types.push_back({ typeName, id, type, redirect });
		m_Types[id].isPrivate = true;

		return m_Types[id];
	}*/
	TypeTableEntry& TypeTable::InsertArray(TypeTableEntry& underlyingType)
	{
		return InsertGeneric(TypeEntryType::Array, { underlyingType });
	}
	TypeTableEntry& TypeTable::InsertTuple(std::vector<TypeTableEntry> underlyingTypes)
	{
		return InsertGeneric(TypeEntryType::Tuple, underlyingTypes);
	}
	TypeTableEntry& TypeTable::InsertFunction(std::vector<TypeTableEntry> argumentTypes, TypeTableEntry returnType)
	{
		std::vector<TypeTableEntry> typeArguments = argumentTypes;
		typeArguments.push_back(returnType);

		return InsertGeneric(TypeEntryType::Function, typeArguments);
	}

	void TypeTable::AddTypeRelation(TypeTableEntry& type, ValueType relatedType, TypeRelation::ConversionType subtypeConversion, TypeRelation::ConversionType supertypeConversion)
	{
		assert(Has(relatedType));

		AddTypeRelation(type, *Lookup(relatedType), subtypeConversion, supertypeConversion);
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

	void TypeTable::Print(std::string padding)
	{
		for (auto& entry : m_Types)
		{
			std::cout << padding << "#" << entry.id << ": " << entry.name << ", "
				<< TypeEntryTypeToString(entry.type);

			if (entry.redirect)
				std::cout << " and is typedef";
			std::cout << "\n";
		}
	}

	void TypeTable::InsertPrimitiveTypes()
	{
		// Insert the build in primitives to the typeEntry table 
		std::vector<std::string> typeKeywords = { "void", "int", "bool", "float", "string" };

		for (uint16_t i = 0; i < typeKeywords.size(); i++)
		{
			m_Typenames[typeKeywords[i]] = i;
			m_Types.push_back({ typeKeywords[i], i, TypeEntryType::Primitive });
		}

		// Insert relation for types
		AddTypeRelation(m_Types[PrimitiveValueTypes::Float], PrimitiveValueTypes::Integer, TypeRelation::Implicit, TypeRelation::Explicit);
		AddTypeRelation(m_Types[PrimitiveValueTypes::Integer], PrimitiveValueTypes::Bool, TypeRelation::Implicit, TypeRelation::Explicit);
	}

	TypeTable::~TypeTable()
	{
	}

}