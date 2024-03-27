#pragma once

#include <string>
#include <unordered_map>
#include <set>
#include <cstdint>

#include "../Operators.h"


namespace O
{
	typedef uint16_t TypeId;
	enum PrimitiveValueTypes
	{
		Void = 0,
		Integer = 1,
		Bool = 2,
		Double = 3,
		String = 4,
	};

	enum class TypeKind
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
		Reference,
		Typedef
	};

	static std::string TypeEntryTypeToString(TypeKind type)
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
		std::string name;
		TypeId id;

		TypeKind kind = TypeKind::Primitive;

		std::vector<TypeId> typeArguments; // Type arguments for generic types

		std::vector<TypeRelation> supertypes;
		std::vector<TypeRelation> subtypes;
		// TODO: Add types at the same level
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

		Type& Insert(const std::string& typeName, TypeKind type);

		Type& InsertGeneric(TypeKind type, std::vector<Type> typeArguments, bool& existed);
		Type& InsertGeneric(TypeKind type, std::vector<Type> typeArguments);

		Type& InsertArray(Type& underlyingType, bool& existed);
		Type& InsertTuple(std::vector<Type> underlyingTypes);
		Type& InsertFunction(std::vector<Type> argumentTypes, Type returnType);
		Type& InsertFunction(std::vector<Type> argumentTypesAndReturnType);

		uint16_t GetAllTypesCount();

		void AddTypeRelation(Type& type, TypeId relatedType, TypeRelation::ConversionType subtypeConversion, TypeRelation::ConversionType supertypeConversion);
		void AddTypeRelation(Type& type, Type& relatedType, TypeRelation::ConversionType subtypeConversion, TypeRelation::ConversionType supertypeConversion);

		std::optional<TypeRelation::ConversionType> GetFullSupertypeRelationTo(Type& type, Type& expectedSupertype);
		std::optional<TypeRelation::ConversionType> GetFullSubtypeRelationTo(Type& type, Type& expectedSubtype);
		std::optional<TypeRelation::ConversionType> GetFullTypeRelationTo(Type& type, Type& expectedType);

		bool IsTypeImplicitSubtypeOf(Type& subtype, Type& expectedSupertype);
		bool AreTypesEquivalent(Type& a, Type& b);

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

		std::unordered_map<uint16_t, Type> m_Types;

		// Perhaps a bad name, but refers to int, float, string etc. 
		std::unordered_map<std::string, TypeId> m_Typenames;
		TypeTable* m_UpwardTypeTable = nullptr;
	};
}