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

	enum class TypeEntryType
	{
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

		TypeEntryType type = TypeEntryType::Primitive;
		TypeTableEntry* redirect = nullptr;

		std::vector<ValueType> typeArguments; // Type arguments for generic types

		std::vector<TypeRelation> supertypes;
		std::vector<TypeRelation> subtypes;

		// A private type is a type that cannot be instantiated like any type
		// TODO: Will be removed once types in the type table are restricted to scopes, 
		// like the symbol table.
		bool isPrivate = false;

		TypeTableEntry& Resolve();
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

		bool Has(const std::string& typeName);
		bool Has(ValueType typeId);

		TypeTableEntry* Lookup(const std::string& typeName);// { return m_Types[m_TypeNames.at(typeName)]; }
		TypeTableEntry* Lookup(ValueType typeId);// { return m_Types[typeId]; }
		//std::optional<TypeTableEntry&> LookupThisTable(std::function<bool(Symbol*)> predicate);

		TypeTableEntry& Insert(const std::string& typeName, TypeEntryType type, TypeTableEntry* redirect = nullptr);

		TypeTableEntry& InsertGeneric(TypeEntryType type, std::vector<TypeTableEntry> typeArguments);
		// const std::string& typeName, TypeTableType type, TypeTableEntry* redirect = nullptr);

		//TypeTableEntry& AddPrivateType(const std::string& typeName, TypeTableType type, TypeTableEntry* redirect = nullptr);

		TypeTableEntry& InsertArray(TypeTableEntry& underlyingType);
		TypeTableEntry& InsertTuple(std::vector<TypeTableEntry> underlyingTypes);
		TypeTableEntry& InsertFunction(std::vector<TypeTableEntry> argumentTypes, TypeTableEntry returnType);

		void AddTypeRelation(TypeTableEntry& type, ValueType relatedType, TypeRelation::ConversionType subtypeConversion, TypeRelation::ConversionType supertypeConversion);
		void AddTypeRelation(TypeTableEntry& type, TypeTableEntry& relatedType, TypeRelation::ConversionType subtypeConversion, TypeRelation::ConversionType supertypeConversion);

		TypeTableEntry& ResolveEntry(TypeTableEntry entry);

		EXPORT const auto& GetTypes() { return m_Types; }
		EXPORT const auto& GetNextTable() { return m_UpwardTypeTable; }

		/*std::vector<VariableSymbol*> LookupVariables(std::string name);
		std::vector<VariableSymbol*> LookupMethods(std::string name);
		std::vector<VariableSymbol*> LookupFunctions(std::string name);*/

		//std::vector<ClassSymbol*> LookupClassesByTypeFirstLayer(ValueType type);

		// Remove the symbol table at scope, and then recursively remove the deeper symbol tables 
		//void Remove(int scope);

		EXPORT void Print(std::string padding);

		EXPORT ~TypeTable();

	private:
		void InsertPrimitiveTypes();

		//Symbol* Insert(Symbol* symbol);

		//void LookupAccumulator(std::function<bool(Symbol*)> predicate, std::vector<Symbol*>& accumulator);
		//void LookupThisTableAccumulator(std::function<bool(Symbol*)> predicate, std::vector<Symbol*>& accumulator);

		//Symbol* One(std::vector<Symbol*> symbols);

		//int GetLargestVariableIndex();

	private:
		TypeTableType m_TableType = TypeTableType::Local;

		std::vector<TypeTableEntry> m_Types;

		// Perhaps a bad name, but refers to int, float, string etc. 
		std::unordered_map<std::string, ValueType> m_Typenames;
		TypeTable* m_UpwardTypeTable = nullptr;
	};
	//public:
	//	TypeTable();

	//	bool HasType(const std::string& typeName) { return m_TypeNames.count(typeName) != 0; }
	//	//bool HasType(ValueType typeId) { return m_T != 0; }
	//	
	//	//ValueType GetType(const std::string& typeName) { return m_TypeNames.at(typeName); }
	//	TypeTableEntry& GetType(const std::string& typeName) { return m_Types[m_TypeNames.at(typeName)]; }
	//	TypeTableEntry& GetType(ValueType typeId) { return m_Types[typeId]; }

	//	TypeTableEntry& Add(const std::string& typeName, TypeTableType type, TypeTableEntry* redirect = nullptr);

	//	TypeTableEntry& AddGeneric(TypeTableType type, std::vector<TypeTableEntry> typeArguments);
	//	// const std::string& typeName, TypeTableType type, TypeTableEntry* redirect = nullptr);

	//	TypeTableEntry& AddPrivateType(const std::string& typeName, TypeTableType type, TypeTableEntry* redirect = nullptr);

	//	TypeTableEntry& AddArray(TypeTableEntry& underlyingType);
	//	TypeTableEntry& AddTuple(std::vector<TypeTableEntry> underlyingTypes);
	//	TypeTableEntry& AddFunction(std::vector<TypeTableEntry> argumentTypes, TypeTableEntry returnType);

	//	void AddTypeRelation(TypeTableEntry& type, ValueType relatedType, TypeRelation::ConversionType subtypeConversion, TypeRelation::ConversionType supertypeConversion);
	//	void AddTypeRelation(TypeTableEntry& type, TypeTableEntry& relatedType, TypeRelation::ConversionType subtypeConversion, TypeRelation::ConversionType supertypeConversion);

	//	TypeTableEntry& ResolveEntry(TypeTableEntry entry);

	//	const auto& AllTypes() { return m_Types; }

	//private:
	//	std::vector<TypeTableEntry> m_Types;
	//	std::unordered_map<std::string, ValueType> m_TypeNames;
	//};
}