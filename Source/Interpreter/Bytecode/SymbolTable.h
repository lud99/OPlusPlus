#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>

#include "../ValueTypes.h"
#include "../../TypeTable.h"

namespace Ö
{
	enum class SymbolType
	{
		Class,
		Function,
		Method,
		Variable
	};

	static std::string SymbolTypeToString(SymbolType type)
	{
		std::string types[] = {
			"Class",
			"Function",
			"Method",
			"Variable"
		};
		return types[(int)type];
	}

	class SymbolTable;
	class SymbolTable
	{
	public:
		struct SymbolAttributes
		{

		};
		class Symbol
		{
		public:
			Symbol() {};
			Symbol(std::string name, SymbolType symbolType, TypeTableEntry* storableValueType);

			//std::size_t operator()(const Symbol& symbol) const noexcept;

			~Symbol() {};

		public:
			std::string m_Name;
			//uint16_t m_Id = 0;

			SymbolType m_SymbolType;
			TypeTableEntry* m_StorableValueType = nullptr;

			TypeTableEntry& GetTypeTableEntry();
		};

		class VariableSymbol : public Symbol
		{
		public:
			using Symbol::Symbol;

			VariableSymbol(std::string name, SymbolType symbolType, TypeTableEntry* storableValueType, uint16_t index);

			uint16_t m_Index;
		};

		class FunctionSymbol : public Symbol
		{
		public:
			using Symbol::Symbol;

			uint16_t m_Id;

			std::vector<ValueType> m_ParameterTypes;
			bool m_IsBuiltIn = false;
		};

		class ClassSymbol;
		class ClassSymbol : public Symbol
		{
		public:
			using Symbol::Symbol;

			//Instructions m_InternalConstructor;
			ClassSymbol* m_ParentClass = nullptr;
			SymbolTable* m_ChildClasses = nullptr;
			SymbolTable* m_Methods = nullptr;
			SymbolTable* m_MemberVariables = nullptr;
			//std::unordered_map<std::string, FunctionSymbol> m_Methods;
			//std::unordered_map<std::string, Symbol> m_MemberVariables;


		};

		SymbolTable() {};
		SymbolTable(SymbolTable* previousSymbolTable, int scope = 0) : 
			m_PreviousSymbolTable(previousSymbolTable), m_Scope(scope) {};

		VariableSymbol* InsertVariable(int scope, std::string name, TypeTableEntry* storeType);
		FunctionSymbol* InsertFunction(int scope, std::string name, TypeTableEntry* returnType, uint16_t id);
		FunctionSymbol* InsertMethod(int scope, std::string name, TypeTableEntry* returnType, uint16_t id);
		ClassSymbol* InsertClass(int scope, std::string name, TypeTableEntry* valueType);

		Symbol* Insert(int scope, std::string name, TypeTableEntry* valueType, SymbolType symbolType, uint16_t id = 0);
		Symbol* Lookup(std::string name);
		SymbolTable::ClassSymbol* LookupClassByType(ValueType type);
		SymbolTable::ClassSymbol* LookupClassByTypeFirstLayer(ValueType type);
		
		// Remove the symbol table at scope, and then recursively remove the deeper symbol tables 
		void Remove(int scope);

		int GetLargestVariableIndex();
		int GetLargestMethodIndex();
	
		bool Has(std::string name);
		bool HasAndIs(std::string name, SymbolType type);

		// Number of symbols in the table (not recursive)
		std::unordered_map<std::string, Symbol*>& GetSymbols() { return m_Symbols; }
		int GetScope() { return m_Scope; }

		~SymbolTable();

	private:
		int m_Scope = 0;

		std::unordered_map<std::string, Symbol*> m_Symbols;
		SymbolTable* m_NextSymbolTable = nullptr;
		SymbolTable* m_PreviousSymbolTable = nullptr;
	};
}
