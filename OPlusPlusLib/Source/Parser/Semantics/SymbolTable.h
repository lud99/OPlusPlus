#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <functional>

#include "TypeTable.h"

namespace O
{
	enum class SymbolType
	{
		Class,
		Function,
		Method,
		Variable
	};

    enum class SymbolTableType
    {
        Local,
        Global
    };

    enum class CallableSymbolType
    {
        Normal,
        Constructor,
        Operator,
    };

    enum class VariableSymbolType
    {
        Local,
        Global,
        Member
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
    class Symbol
    {
    public:
        Symbol() {};
        Symbol(std::string name, SymbolType symbolType, ValueType dataType);

    public:
        std::string m_Name;
        SymbolType m_SymbolType;
        ValueType m_DataType;

        bool operator==(const Symbol& other);

        //TypeTableEntry& GetTypeTableEntry();
    };

    class VariableSymbol : public Symbol
    {
    public:
        using Symbol::Symbol;
        VariableSymbol(std::string name, ValueType dataType, uint16_t index, VariableSymbolType variableType);

        // Does not compare the indicies
        bool operator==(const VariableSymbol& other);
        
        uint16_t m_Index = 0;
        VariableSymbolType m_VariableType = VariableSymbolType::Local;
    };

    class CallableSymbol : public Symbol
    {
    public:
        using Symbol::Symbol;
        CallableSymbol(std::string name, SymbolType symbolType, ValueType returnType, uint16_t id, CallableSymbolType callableType);

        // Compares parameters, isBuiltIn and functionType (and default)
        bool operator==(const CallableSymbol& other);

    public:
        uint16_t m_Id;

        std::vector<ValueType> m_ParameterTypes;
        // TODO: Add data structure for default types
        
        bool m_IsBuiltIn = false;
        CallableSymbolType m_CallableType = CallableSymbolType::Normal;
    };

    class ClassSymbol;
    class ClassSymbol : public Symbol
    {
    public:
        using Symbol::Symbol;
        ClassSymbol(std::string name, ValueType dataType);

        ~ClassSymbol();

    public:
        SymbolTable* m_Symbols;
    };

	class SymbolTable;
	class SymbolTable
	{
	public:
		SymbolTable();
		SymbolTable(SymbolTableType tableType, SymbolTable* upwardSymbolTable);

		VariableSymbol* InsertVariable(std::string name, ValueType dataType, VariableSymbolType variableType);
		CallableSymbol* InsertCallable(CallableSymbol callable);
		ClassSymbol* InsertClass(std::string name, ValueType dataType);

		std::vector<Symbol*> Lookup(std::function<bool(Symbol*)> predicate);
		std::vector<Symbol*> LookupThisTable(std::function<bool(Symbol*)> predicate);

		std::vector<Symbol*> Lookup(std::string name);
		std::vector<Symbol*> Lookup(Symbol* symbol);

		Symbol* LookupOne(std::string name);

		/*std::vector<VariableSymbol*> LookupVariables(std::string name);
		std::vector<VariableSymbol*> LookupMethods(std::string name);
		std::vector<VariableSymbol*> LookupFunctions(std::string name);*/


		ClassSymbol* LookupClassByType(ValueType type);
		//std::vector<ClassSymbol*> LookupClassesByTypeFirstLayer(ValueType type);
		
		// Remove the symbol table at scope, and then recursively remove the deeper symbol tables 
		//void Remove(int scope);

		bool Has(std::string name);
		bool HasAndIs(std::string name, SymbolType type);

		auto& GetSymbols() { return m_Symbols; }

        ~SymbolTable();

    private:
        Symbol* Insert(Symbol* symbol);
        
        void LookupAccumulator(std::function<bool(Symbol*)> predicate, std::vector<Symbol*>& accumulator);
        void LookupThisTableAccumulator(std::function<bool(Symbol*)> predicate, std::vector<Symbol*>& accumulator);

        Symbol* One(std::vector<Symbol*> symbols);

        int GetLargestVariableIndex();

	private:
        SymbolTableType m_TableType = SymbolTableType::Local;

		std::vector<std::vector<Symbol*>> m_Symbols;
		SymbolTable* m_UpwardSymbolTable = nullptr;
	};
}
