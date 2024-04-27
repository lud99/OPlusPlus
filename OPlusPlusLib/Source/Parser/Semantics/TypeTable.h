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
		Primitive, // All the others except primitive are template types
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

	class TypeTable;
	class Type;
	class Type
	{
	public:
		Type() {};
		Type(const std::string& typeName, TypeId id, TypeKind kind, std::vector<TypeId> typeArguments = {});

		TypeId id;

		TypeKind kind = TypeKind::Primitive;

		std::vector<TypeId> typeArguments; // Type arguments for generic types

		std::vector<TypeRelation> supertypes;
		std::vector<TypeRelation> subtypes;
		// TODO: Add types at the same level

	private:
		std::string typeName; // Name for primitives and declared classes

	public:
		std::string GetName(TypeTable* table) const;
		std::string GetName(const TypeTable* table) const;
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

		const Type* Lookup(const std::string& typeName);
		const Type* Lookup(TypeId typeId);
		std::vector<const Type*> Lookup(std::vector<TypeId> typeIds);
		const Type* LookupReference(TypeId typeId);

		const Type* Insert(const std::string& typeName, TypeKind type, bool insertReference = true);

		const Type* InsertGeneric(TypeKind type, std::vector<const Type*> typeArguments, bool& existed, bool insertReference = true);
		const Type* InsertGeneric(TypeKind type, std::vector<const Type*> typeArguments, bool insertReference = true);

		const Type* InsertIncomplete();

		// TODO: refactor to use assignment overload in the class instead?
		const Type* Replace(const Type* type, const Type* newType);

		const Type* InsertArray(const Type* underlyingType, bool& existed);
		const Type* InsertTuple(std::vector<const Type*> underlyingTypes);
		const Type* InsertFunction(std::vector<const Type*> argumentTypes, const Type* returnType);
		const Type* InsertFunction(std::vector<const Type*> argumentTypesAndReturnType);


		void AddTypeRelation(Type* type, TypeId relatedType, TypeRelation::ConversionType subtypeConversion, TypeRelation::ConversionType supertypeConversion);
		void AddTypeRelation(Type* type, Type* relatedType, TypeRelation::ConversionType subtypeConversion, TypeRelation::ConversionType supertypeConversion);

		std::optional<TypeRelation::ConversionType> GetFullSupertypeRelationTo(const Type* type, const Type* expectedSupertype);
		std::optional<TypeRelation::ConversionType> GetFullSubtypeRelationTo(const Type* type, const Type* expectedSubtype);
		std::optional<TypeRelation::ConversionType> GetFullTypeRelationTo(const Type* type, const Type* expectedType);

		// Also checks if the types are equal
		bool IsTypeImplicitSubtypeOf(const Type* subtype, const Type* expectedSupertype);

		bool AreTypesEquivalent(const Type* a, const Type* b);
		bool AreTypesEquivalent(TypeId a, TypeId b);

		// If the type relations is seen like a tree, then this function returns the height of the tree from this node
		uint16_t GetHeightOfTypeRelation(const Type* type);

		EXPORT const auto& GetTypes() { return m_Types; }
		EXPORT const auto& GetNextTable() { return m_UpwardTypeTable; }

		EXPORT void Print(std::string padding);

		EXPORT ~TypeTable();

	private:
		void InsertPrimitiveTypes();
		const Type* InsertReferenceType(const Type* type);

		Type* LookupNonConst(TypeId typeId);

		TypeId GetNextFreeTypeId();

	private:
		TypeTableType m_TableType = TypeTableType::Local;

		std::unordered_map<TypeId, Type*> m_Types;

		// Perhaps a bad name, but refers to int, float, string etc. 
		std::unordered_map<std::string, TypeId> m_Typenames;
		TypeTable* m_UpwardTypeTable = nullptr;

		static TypeId m_NextFreeTypeId;
	};
}