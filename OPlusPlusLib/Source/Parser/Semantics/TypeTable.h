#pragma once

#include <string>
#include <unordered_map>
#include <set>
#include <cstdint>

#include "../Operators.h"

namespace O
{
	enum PrimitiveValueTypes
	{
		Void = 0,
		Integer = 1,
		Bool = 2,
		Float = 3,
		String = 4,
	};
	typedef uint16_t ValueType;

	enum class TypeTableType
	{
		Class,
		Function,
		Method,
		Primitive,
		Array,
		Nullable,
		Typedef
	};

	static std::string TypeTableTypeToString(TypeTableType type)
	{
		return std::string(magic_enum::enum_name(type));
	}

	struct TypeRelation
	{
		enum ConversionType
		{
			Implicit,
			Explicit
		};

		ConversionType conversionType;
		ValueType relatedType;
	};

	class TypeTableEntry;
	class TypeTableEntry
	{
	public:
		// typeEntry: built in, object ref, array, typedef
		// redirect: pointer to actual typeEntry entry, if a typedef
		// id: id

		std::string name;
		ValueType id;

		TypeTableType type = TypeTableType::Primitive;
		TypeTableEntry* redirect = nullptr;

		ValueType underlyingType; // for arrays

		std::vector<TypeRelation> supertypes;
		std::vector<TypeRelation> subtypes;

		// A private type is a type that cannot be instantiated like any type
		// TODO: Will be removed once types in the type table are restricted to scopes, 
		// like the symbol table.
		bool isPrivate = false;

		TypeTableEntry& Resolve();
	};

	class TypeTable
	{
	public:
		TypeTable();

		bool HasType(const std::string& typeName) { return m_TypeNames.count(typeName) != 0; }
		
		ValueType GetType(const std::string& typeName) { return m_TypeNames.at(typeName); }
		TypeTableEntry& GetTypeEntry(const std::string& typeName) { return m_Types[m_TypeNames.at(typeName)]; }

		TypeTableEntry& GetEntryFromId(ValueType id);

		TypeTableEntry& Add(const std::string& typeName, TypeTableType type, TypeTableEntry* redirect = nullptr);
		TypeTableEntry& AddPrivateType(const std::string& typeName, TypeTableType type, TypeTableEntry* redirect = nullptr);

		ValueType AddArray(ValueType underlyingType);

		void AddTypeRelation(TypeTableEntry& type, ValueType relatedType, TypeRelation::ConversionType subtypeConversion, TypeRelation::ConversionType supertypeConversion);
		void AddTypeRelation(TypeTableEntry& type, TypeTableEntry& relatedType, TypeRelation::ConversionType subtypeConversion, TypeRelation::ConversionType supertypeConversion);

		TypeTableEntry& ResolveEntry(TypeTableEntry entry);

		//const std::unordered_map<std::string, TypeTableEntry>& AllTypes() { return m_Types; }

	private:
		std::vector<TypeTableEntry> m_Types;
		std::unordered_map<std::string, ValueType> m_TypeNames;
	};
}