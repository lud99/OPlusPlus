#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>

namespace Ö
{
	enum PrimitiveValueTypes
	{
		Void = 0,
		Integer = 1,
		Float = 2,
		String = 3,
	};
	typedef uint16_t ValueType;

	enum class TypeTableType
	{
		Class,
		Function,
		Method,
		Primitive,
		Typedef
	};

	static std::string TypeTableTypeToString(TypeTableType type)
	{
		std::string types[] = {
			"Class",
			"Function",
			"Method",
			"Primitive",
			"Typedef"
		};
		return types[(int)type];
	}

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

		// A private type is a type that cannot be instantiated like any type
		bool isPrivate = false;

		TypeTableEntry& Resolve();
	};

	class TypeTable
	{
	public:
		TypeTable();

		bool HasType(const std::string& typeName) { return m_Types.count(typeName) != 0; }
		
		ValueType GetType(const std::string& typeName) { return GetTypeEntry(typeName).id; }
		TypeTableEntry& GetTypeEntry(const std::string& typeName) { return m_Types.at(typeName); }


		TypeTableEntry& GetEntryFromId(ValueType id);

		TypeTableEntry& Add(const std::string& typeName, TypeTableType type, TypeTableEntry* redirect = nullptr);
		TypeTableEntry& AddPrivateType(const std::string& typeName, TypeTableType type, TypeTableEntry* redirect = nullptr);

		TypeTableEntry& ResolveEntry(TypeTableEntry entry);

		const std::unordered_map<std::string, TypeTableEntry>& AllTypes() { return m_Types; }

	private:
		std::unordered_map<std::string, TypeTableEntry> m_Types;

		ValueType m_CurrentFreeTypeId = 0;
	};
}