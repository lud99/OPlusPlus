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

	bool TypeTable::HasType(const std::string& typeName)
	{
		// First look in the current table
		if (m_Typenames.count(typeName) != 0)
			return true;

		// Otherwise look upward
		if (m_UpwardTypeTable)
			return m_UpwardTypeTable->HasType(typeName);

		assert(m_TableType == TypeTableType::Global);
		return false;
	}

	bool TypeTable::HasType(ValueType typeId)
	{
		// First look in the current table
		for (auto& type : m_Types)
		{
			if (type.id == typeId)
				return true;
		}

		// Otherwise look upward
		if (m_UpwardTypeTable)
			return m_UpwardTypeTable->HasType(typeId);

		assert(m_TableType == TypeTableType::Global);
		return false;
	}

	bool TypeTable::HasCompleteType(const std::string& typeName)
	{
		// First look in the current table
		if (m_Typenames.count(typeName) != 0)
			return m_Types[m_Typenames[typeName]].type != TypeEntryType::Incomplete;

		// Otherwise look upward
		if (m_UpwardTypeTable)
			return m_UpwardTypeTable->HasCompleteType(typeName);

		assert(m_TableType == TypeTableType::Global);
		return false;
	}

	bool TypeTable::HasCompleteType(ValueType typeId)
	{
		// First look in the current table
		for (auto& type : m_Types)
		{
			if (type.id == typeId)
				return type.type != TypeEntryType::Incomplete;
		}

		// Otherwise look upward
		if (m_UpwardTypeTable)
			return m_UpwardTypeTable->HasCompleteType(typeId);

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
		assert(!HasCompleteType(typeName));

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

		if (HasCompleteType(name))
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
		assert(HasCompleteType(relatedType));

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

	std::optional<TypeRelation::ConversionType> TypeTable::GetFullSupertypeRelationTo(TypeTableEntry& type, TypeTableEntry& expectedSupertype)
	{
		for (auto& typeRelation : type.supertypes)
		{
			TypeTableEntry* supertype = Lookup(typeRelation.relatedType);
			assert(supertype);

			if (supertype->id == expectedSupertype.id)
				return typeRelation.conversionType;

			// Continue searching upwards
			auto upwardTypeRelation = GetFullSupertypeRelationTo(*supertype, expectedSupertype);
			if (upwardTypeRelation.has_value())
			{
				// If this relation and the above are both implicit, or explicit then dont modify it
				if (typeRelation.conversionType == upwardTypeRelation)
					return upwardTypeRelation.value();

				// Otherwise it is explicit somewhere in the chain, so the total relation is explicit
				return TypeRelation::Explicit;
			}
		}

		return {};
	}

	std::optional<TypeRelation::ConversionType> TypeTable::GetFullSubtypeRelationTo(TypeTableEntry& type, TypeTableEntry& expectedSubtype)
	{
		for (auto& typeRelation : type.subtypes)
		{
			TypeTableEntry* subtype = Lookup(typeRelation.relatedType);
			assert(subtype);

			if (subtype->id == expectedSubtype.id)
				return typeRelation.conversionType;

			// Continue searching downwards
			auto downwardTypeRelation = GetFullSubtypeRelationTo(*subtype, expectedSubtype);
			if (downwardTypeRelation.has_value())
			{
				// If this relation and the above are both implicit, or explicit then dont modify it
				if (typeRelation.conversionType == downwardTypeRelation)
					return downwardTypeRelation.value();

				// Otherwise it is explicit somewhere in the chain, so the total relation is explicit
				return TypeRelation::Explicit;
			}
		}

		return {};
	}

	std::optional<TypeRelation::ConversionType> TypeTable::GetFullTypeRelationTo(TypeTableEntry& type, TypeTableEntry& expectedType)
	{
		auto supertypeRelation = GetFullSupertypeRelationTo(type, expectedType);
		auto subtypeRelation = GetFullSubtypeRelationTo(type, expectedType);

		// Can't be both sub and super type at the same time
		if (subtypeRelation.has_value() && supertypeRelation.has_value())
			abort();

		if (subtypeRelation.has_value())
			return subtypeRelation;
		if (supertypeRelation.has_value())
			return supertypeRelation;
		
		return {};
	}

	bool TypeTable::IsTypeImplicitSubtypeOf(TypeTableEntry& subtype, TypeTableEntry& expectedSupertype)
	{
		for (auto& typeRelation : subtype.supertypes)
		{
			TypeTableEntry* supertype = Lookup(typeRelation.relatedType);
			assert(supertype);

			if (supertype->id == expectedSupertype.id)
				return typeRelation.conversionType == TypeRelation::Implicit;

			// Continue searching upwards
			if (IsTypeImplicitSubtypeOf(*supertype, expectedSupertype))
				return typeRelation.conversionType == TypeRelation::Implicit;
		}

		return false;
	}

	uint16_t TypeTable::GetHeightOfTypeRelation(TypeTableEntry& type)
	{
		uint16_t highestRelation = 0;
		for (TypeRelation& relation : type.subtypes)
		{
			auto typeEntry = Lookup(relation.relatedType);
			assert(typeEntry);

			uint16_t height = GetHeightOfTypeRelation(*typeEntry) + 1;
			if (height >= highestRelation)
				highestRelation = height;
		}

		return highestRelation;
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
		std::vector<std::string> typeKeywords = { "void", "int", "bool", "double", "string" };

		for (uint16_t i = 0; i < typeKeywords.size(); i++)
		{
			m_Typenames[typeKeywords[i]] = i;
			m_Types.push_back({ typeKeywords[i], i, TypeEntryType::Primitive });
		}

		// Insert relation for types
		// double -> int (explicit)
		// int -> double (implicit)
		AddTypeRelation(m_Types[PrimitiveValueTypes::Double], PrimitiveValueTypes::Integer, TypeRelation::Explicit, TypeRelation::Implicit);
		
		// int -> bool (explicit)
		// bool -> int (explicit)
		AddTypeRelation(m_Types[PrimitiveValueTypes::Integer], PrimitiveValueTypes::Bool, TypeRelation::Explicit, TypeRelation::Explicit);
	}

	TypeTable::~TypeTable()
	{
	}

}