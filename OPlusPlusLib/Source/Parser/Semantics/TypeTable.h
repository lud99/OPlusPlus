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
		Double = 3,
		String = 4,
	};
	typedef uint16_t TypeId;

	enum class TypeEntryType
	{
		Incomplete,
		Error,

		Class,
		Function,
		Method,
		Primitive,
		Array,
		Tuple,
		Nullable,
		Typedef
	};

	static std::string TypeEntryTypeToString(TypeEntryType type)
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
		TypeId relatedType;
	};

	class Type;
	class Type
	{
	public:
		// typeEntry: built in, object ref, array, typedef
		// redirect: pointer to actual typeEntry entry, if a typedef
		// id: id

		std::string name;
		TypeId id;

		TypeEntryType type = TypeEntryType::Primitive;

		std::vector<TypeId> typeArguments; // Type arguments for generic types

		std::vector<TypeRelation> supertypes;
		std::vector<TypeRelation> subtypes;
		// TODO: Add types at the same level

		// 
		//std::unordered_map<Operators::Name, Type*> definedOperators;

		// A private type is a type that cannot be instantiated like any type
		// TODO: Will be removed once types in the type table are restricted to scopes, 
		// like the symbol table.
		//bool isPrivate = false;
	};



	enum class TypeTableType
	{
		Local,
		Global
	};

	class TypeTable;
	class TypeTable
	{
	public:
		TypeTable();
		EXPORT TypeTable(TypeTableType tableType, TypeTable* upwardTypeTable);

		bool HasType(const std::string& typeName);
		bool HasType(TypeId typeId);
		bool HasCompleteType(const std::string& typeName);
		bool HasCompleteType(TypeId typeId);

		Type* Lookup(const std::string& typeName);
		Type* Lookup(TypeId typeId);

		Type& Insert(const std::string& typeName, TypeEntryType type);

		Type& InsertGeneric(TypeEntryType type, std::vector<Type> typeArguments);

		Type& InsertArray(Type& underlyingType);
		Type& InsertTuple(std::vector<Type> underlyingTypes);
		Type& InsertFunction(std::vector<Type> argumentTypes, Type returnType);

		void AddTypeRelation(Type& type, TypeId relatedType, TypeRelation::ConversionType subtypeConversion, TypeRelation::ConversionType supertypeConversion);
		void AddTypeRelation(Type& type, Type& relatedType, TypeRelation::ConversionType subtypeConversion, TypeRelation::ConversionType supertypeConversion);

		std::optional<TypeRelation::ConversionType> GetFullSupertypeRelationTo(Type& type, Type& expectedSupertype);
		std::optional<TypeRelation::ConversionType> GetFullSubtypeRelationTo(Type& type, Type& expectedSubtype);
		std::optional<TypeRelation::ConversionType> GetFullTypeRelationTo(Type& type, Type& expectedType);

		bool IsTypeImplicitSubtypeOf(Type& subtype, Type& expectedSupertype);

		// If the type relations is seen like a tree, then this function returns the height of the tree from this node
		uint16_t GetHeightOfTypeRelation(Type& type);

		EXPORT const auto& GetTypes() { return m_Types; }
		EXPORT const auto& GetNextTable() { return m_UpwardTypeTable; }

		EXPORT void Print(std::string padding);

		EXPORT ~TypeTable();

	private:
		void InsertPrimitiveTypes();

	private:
		TypeTableType m_TableType = TypeTableType::Local;

		std::vector<Type> m_Types;

		// Perhaps a bad name, but refers to int, float, string etc. 
		std::unordered_map<std::string, TypeId> m_Typenames;
		TypeTable* m_UpwardTypeTable = nullptr;
	};
}