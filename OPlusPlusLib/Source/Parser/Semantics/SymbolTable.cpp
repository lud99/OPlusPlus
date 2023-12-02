#include "SymbolTable.h"

#include <assert.h>
#include <iostream>

namespace O
{
	// Type& SymbolTable::Symbol::GetTypeTableEntry()
	// {
	// 	return m_StorableValueType->Resolve();
	// }

	Symbol::Symbol(std::string name, SymbolType symbolType, TypeId dataType)
	{
		m_Name = name;
		m_SymbolType = symbolType;
		m_DataType = dataType;
	}

    bool Symbol::operator==(const Symbol &other)
    {
        return m_Name == other.m_Name &&
            m_SymbolType == other.m_SymbolType &&
            m_DataType == other.m_DataType;
    }

    VariableSymbol::VariableSymbol(std::string name, TypeId dataType, uint16_t index, VariableSymbolType variableType)
	{
        m_SymbolType = SymbolType::Variable;

		m_Name = name;
		m_DataType = dataType;
		
		m_Index = index;
        m_VariableType = variableType;
	}

    bool VariableSymbol::operator==(const VariableSymbol &other)
    {
        return m_Name == other.m_Name;
    }

    CallableSymbol::CallableSymbol(std::string name, SymbolType symbolType, TypeId returnType, uint16_t id, CallableSymbolType callableType)
	{
		m_Name = name;
		m_SymbolType = symbolType;
		m_DataType = returnType;

		m_Id = id;
        m_CallableType = callableType;
	}

    bool CallableSymbol::operator==(const CallableSymbol &other)
    {
        return m_Name == other.m_Name &&
            // method == function?. m_SymbolType == other.m_SymbolType &&
            // TODO: look into overloading of returnType. m_DataType == other.m_DataType &&
            // unique for callable
            m_ParameterTypes == other.m_ParameterTypes;
            //m_IsBuiltIn == other.m_IsBuiltIn &&
            //m_CallableType == other.m_CallableType;
    }

    /*std::size_t SymbolTable::Symbol::operator()(const Symbol& symbol) const noexcept
	{
		return m_Id;
	}*/

    
    ClassSymbol::ClassSymbol(std::string name, TypeId dataType, SymbolTable* upwardSymbolTable, TypeTable* upwardTypeTable)
    {
        m_SymbolType = SymbolType::Class;
        
        m_Name = name;
        m_DataType = dataType;

        m_Symbols = new SymbolTable(SymbolTableType::Local, upwardSymbolTable);
        m_Types = new TypeTable(TypeTableType::Local, upwardTypeTable);
    }

    ClassSymbol::~ClassSymbol()
    {
        delete m_Symbols;
        delete m_Types;
    }

    SymbolTable::SymbolTable()
    {
        m_TableType = SymbolTableType::Global;
    }

    SymbolTable::SymbolTable(SymbolTableType tableType, SymbolTable* upwardSymbolTable)
    {
        m_TableType = tableType;
        m_UpwardSymbolTable = upwardSymbolTable;
    }

    VariableSymbol* SymbolTable::InsertVariable(std::string name, TypeId dataType, VariableSymbolType variableType)
	{
        uint16_t index = GetLargestVariableIndex() + 1;
		return (VariableSymbol*)Insert(new VariableSymbol(name, dataType, index, variableType));
	}

	CallableSymbol* SymbolTable::InsertCallable(CallableSymbol callable)
	{
        return (CallableSymbol*)Insert(new CallableSymbol(callable));
	}

	ClassSymbol* SymbolTable::InsertClass(std::string name, TypeId dataType, SymbolTable* upwardSymbolTable, TypeTable* upwardTypeTable)
	{
		ClassSymbol* classSymbol = (ClassSymbol*)Insert(new ClassSymbol(name, dataType, upwardSymbolTable, upwardTypeTable));
		
		return classSymbol;
	}

    std::vector<Symbol*> SymbolTable::LookupAny(std::function<bool(Symbol*)> predicate)
    {
        std::vector<Symbol*> result;
        LookupAccumulator(predicate, result);
        return result;
    }

    /*std::vector<Symbol*> SymbolTable::LookupThisTable(std::function<bool(Symbol*)> predicate)
    {
        std::vector<Symbol*> result;
        LookupThisTableAccumulator(predicate, result);
        return result;
    }*/

    std::vector<Symbol*> SymbolTable::Lookup(std::string name)
	{
        // Lookup this table
        if (m_Symbols.count(name) != 0)
            return m_Symbols[name];

        // Look upward
        if (m_UpwardSymbolTable)
            return m_UpwardSymbolTable->Lookup(name);

        assert(m_TableType == SymbolTableType::Global);
        return {};
	}

    std::vector<Symbol*> SymbolTable::Lookup(Symbol* symbol)
    {
        assert(symbol);
     
        // Lookup this table
        if (m_Symbols.count(symbol->m_Name) != 0)
        {
            // Check the symbols with the same name if they are identical to the specified symbol
            auto& symbols = m_Symbols[symbol->m_Name];
            for (Symbol* itSymbol : symbols)
            {
                if (*itSymbol == *symbol)
                    return symbols;
            }
        }

        // Look upward
        if (m_UpwardSymbolTable)
            return m_UpwardSymbolTable->Lookup(symbol);

        assert(m_TableType == SymbolTableType::Global);
        return {};
    }

	Symbol* SymbolTable::LookupOne(std::string name)
	{
		auto symbols = Lookup(name);
		if (symbols.empty())
			return nullptr;
		
		return symbols[0];
	}

	ClassSymbol* SymbolTable::LookupClassByType(TypeId type)
	{
        for (auto& [name, symbols] : m_Symbols)
        {
            for (Symbol* symbol : symbols)
            {
                if (symbol->m_DataType == type)
                {
                    assert(symbols.size() <= 1);
                    return (ClassSymbol*)One(symbols);
                }
            }
        }
        
        return nullptr;
	}

    bool SymbolTable::Has(std::string name)
	{
		return !Lookup(name).empty();
	}

	bool SymbolTable::HasAndIs(std::string name, SymbolType type)
	{
		if (!Has(name)) return false;

		return LookupOne(name)->m_SymbolType == type;
	}

    bool SymbolTable::Has(Symbol* symbol)
    {
        return !Lookup(symbol).empty();
    }

    Symbol* SymbolTable::Insert(Symbol* symbol)
	{
		// Make sure the *exact* symbol hasn't been declared before (uses the overloaded comparison operators on the symbols)
		assert(!Has(symbol));
        
        m_Symbols[symbol->m_Name].push_back(symbol);

        return symbol;
	}

    void SymbolTable::LookupAccumulator(std::function<bool(Symbol*)> predicate, std::vector<Symbol*>& accumulator)
    {
		// First search in the current symbol table
        LookupThisTableAccumulator(predicate, accumulator);

		// Otherwise, resursively search the upward symbol table in the scope above
        if (m_UpwardSymbolTable != nullptr)
            m_UpwardSymbolTable->LookupAccumulator(predicate, accumulator);
    }

    void SymbolTable::LookupThisTableAccumulator(std::function<bool(Symbol*)> predicate, std::vector<Symbol*>& accumulator)
    {
        std::vector<Symbol*> result;

        for (auto& [name, symbols] : m_Symbols)
        {
            for (auto symbol : symbols)
            {
                if (predicate(symbol))
                    accumulator.push_back(symbol);
            }
        }
    }

    Symbol* SymbolTable::One(std::vector<Symbol*> symbols)
    {
        if (symbols.empty()) 
            return nullptr;
        
        return symbols[0];
    }

	// Recursively find the largest variable index in the symbol table above
	int SymbolTable::GetLargestVariableIndex()
	{
		int largestIndex = -1;
		if (m_UpwardSymbolTable) 
			largestIndex = m_UpwardSymbolTable->GetLargestVariableIndex();

        for (auto [name, symbolsWithName] : m_Symbols)
        {
            for (auto symbol : symbolsWithName)
            {
                if (symbol->m_SymbolType != SymbolType::Variable)
                    continue;

                VariableSymbol* variableSymbol = (VariableSymbol*)symbol;

                if (variableSymbol->m_Index > largestIndex)
                    largestIndex = variableSymbol->m_Index;
            }
        }

		return largestIndex;
	}

    void SymbolTable::Print(TypeTable& localTypeTable, std::string padding)
    {
        for (auto& [name, symbols] : m_Symbols)
        {
            for (auto symbol : symbols)
            {
                Type* dataType = localTypeTable.Lookup(symbol->m_DataType);
                assert(dataType);

                std::cout << padding << symbol->m_Name;

                if (symbol->m_SymbolType == SymbolType::Function || symbol->m_SymbolType == SymbolType::Method)
                {
                    std::cout << " (";
                    CallableSymbol* callableSymbol = (CallableSymbol*)symbol;
                    for (int i = 0; i < callableSymbol->m_ParameterTypes.size(); i++)
                    {
                        TypeId parameterTypeId = callableSymbol->m_ParameterTypes[i];

                        Type* type = localTypeTable.Lookup(parameterTypeId);
                        assert(type);

                        std::cout << type->name;
                        if (i < callableSymbol->m_ParameterTypes.size() - 1)
                            std::cout << ", ";
                    }
                    std::cout << ")";
                }

                std::cout << ": " << dataType->name;

                std::cout << ", " << SymbolTypeToString(symbol->m_SymbolType) << "\n";
            }
        }
    }

    SymbolTable::~SymbolTable()
	{
        for (auto& [name, symbols] : m_Symbols)
        {
            for (auto symbol : symbols)
                delete symbol;
        }
	}
}
