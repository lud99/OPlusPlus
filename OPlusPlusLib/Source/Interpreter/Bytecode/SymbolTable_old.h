#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>

#include "../ValueTypes.h"
#include "../../TypeTable.h"

namespace �
{
	enum class SymbolType
	{
		Class,
		Function,
		Method,
		Variable
	};

	// TODO: Add for other symbols aswell
	struct SymbolName
	{
		std::string name;
		std::vector<ValueType> parameterTypes;

		SymbolName() {};
		SymbolName(std::string n) : name(n) {};

		std::size_t operator()(const SymbolName& symbol) const noexcept
		{
			std::string fullName = name + " ";
			for (auto& param : parameterTypes)
				fullName += std::to_string(param);

			return std::hash<std::string>{}(fullName);
		}
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

		enum class FunctionType
		{
			Normal,
			Constructor,
			Operator,
		};

		class FunctionSymbol : public Symbol
		{
		public:
			FunctionSymbol(std::string name, SymbolType symbolType, TypeTableEntry* storableValueType, uint16_t id);

			uint16_t m_Id;

			std::vector<ValueType> m_ParameterTypes;
			bool m_IsBuiltIn = false;

			FunctionType m_FunctionType = FunctionType::Normal;
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
			, m_Scope(scope) {};

		VariableSymbol* InsertVariable(int scope, std::string name, TypeTableEntry* storeType);
		FunctionSymbol* InsertFunction(int scope, std::string name, TypeTableEntry* returnType, uint16_t id);
		FunctionSymbol* InsertMethod(int scope, std::string name, TypeTableEntry* returnType, uint16_t id);
		ClassSymbol* InsertClass(int scope, std::string name, TypeTableEntry* valueType);

		Symbol* Insert(int scope, std::string name, TypeTableEntry* valueType, SymbolType symbolType, uint16_t id = 0);

		std::vector<Symbol*> Lookup(std::string name);
		Symbol* LookupOne(std::string name);

		SymbolTable::ClassSymbol* LookupClassByType(ValueType type);
		SymbolTable::ClassSymbol* LookupClassByTypeFirstLayer(ValueType type);
		
		// Remove the symbol table at scope, and then recursively remove the deeper symbol tables 
		void Remove(int scope);

		int GetLargestVariableIndex();
	
		bool Has(std::string name);
		bool HasAndIs(std::string name, SymbolType type);

		auto& GetSymbols() { return m_Symbols; }
		int GetScope() { return m_Scope; }

		~SymbolTable();

	private:
		int m_Scope = 0;

		std::vector<std::vector<Symbol*>> m_Symbols;
		//SymbolTable* m_NextSymbolTable = nullptr;
		SymbolTable* m_SymbolTableInParentScope = nullptr;
	};
}
