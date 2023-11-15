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


		std::set<ValueType> supertypes;
		std::set<ValueType> subtypes;

		// Set of types which this type has casts to
		/*std::set<TypeTableEntry*> implicitTypecasts;
		std::set<TypeTableEntry*> explicitTypecasts;

		std::unordered_map<Operators::Operator, TypeTableEntry*> */

		// int +(int other)

		// destructuring
		// [1, 2] = (1,2) or
		// (1, 2) = (1, 2);
		// { int age, string name } = Animal()

		// a[0] -> a `subscript` 0
		// a[0, 1, 2] -> a `subscript` (0, 1, 2)
		// f (1, 2, 3); int f(int a, int b, int c) // destructuring the input tuple
		// f (1, 2, 3); int f ((int, int, int) tuple) // destructuring the input tuple
		// f (5); int f (int a) // function def has to destructure arguments tuple
		// otherwise: int f (int) a => 
		// ..and 1 + (2 + 3), is (2 + 3) a tuple? if not, then is this? f (5)

		// instead of parsing tuples as default, call them "grouped expressions"
		// that would solve all problems
			// ',' is grouping operator. 5 + 2, 8 + 9. has the highest repcedence
			// not a tuple, but a group. so a func def has a group of parameters
			// and the same with function call f(1, 2, 3).
			// a tuple is when a () is placed. remember '(' is a binary call operator

		// (5, 5) is tuple. (5) is groupExpr
		// (int a, int b) is group. (a, b) is tuple

		// int + int. int["+"] = int add(int other) =>
		// float + int, ej definierat.
		// men om int implicit castas som float blir den definierat
		// string + int, nej och int har ingen implicit cast
		// string + int.ToString()

		// type Name 
		// Name + ... då vill vi resolva name till den underliggande typen

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

		bool HasType(const std::string& typeName) { return m_Types.count(typeName) != 0; }
		
		ValueType GetType(const std::string& typeName) { return GetTypeEntry(typeName).id; }
		TypeTableEntry& GetTypeEntry(const std::string& typeName) { return m_Types.at(typeName); }

		TypeTableEntry& GetEntryFromId(ValueType id);

		TypeTableEntry& Add(const std::string& typeName, TypeTableType type, TypeTableEntry* redirect = nullptr);
		TypeTableEntry& AddPrivateType(const std::string& typeName, TypeTableType type, TypeTableEntry* redirect = nullptr);

		void AddSubtypeRelation(TypeTableEntry& type, ValueType subtypeId);

		TypeTableEntry& ResolveEntry(TypeTableEntry entry);

		const std::unordered_map<std::string, TypeTableEntry>& AllTypes() { return m_Types; }

	private:
		std::unordered_map<std::string, TypeTableEntry> m_Types;

		ValueType m_CurrentFreeTypeId = 0;
	};
}