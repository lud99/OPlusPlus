#include "SymbolTable.h"

#include <assert.h>

namespace Ö
{
	// TypeTableEntry& SymbolTable::Symbol::GetTypeTableEntry()
	// {
	// 	return m_StorableValueType->Resolve();
	// }

	Symbol::Symbol(std::string name, SymbolType symbolType, ValueType dataType)
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

    VariableSymbol::VariableSymbol(std::string name, ValueType dataType, uint16_t index, VariableSymbolType variableType)
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

    CallableSymbol::CallableSymbol(std::string name, SymbolType symbolType, ValueType returnType, uint16_t id, CallableSymbolType callableType)
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
            m_ParameterTypes == other.m_ParameterTypes &&
            //m_IsBuiltIn == other.m_IsBuiltIn &&
            //m_CallableType == other.m_CallableType;
    }

    /*std::size_t SymbolTable::Symbol::operator()(const Symbol& symbol) const noexcept
	{
		return m_Id;
	}*/

    
    ClassSymbol::ClassSymbol(std::string name, ValueType dataType)
    {
        m_SymbolType = SymbolType::Class;
        
        m_Name = name;
        m_DataType = dataType;
    }

    ClassSymbol::~ClassSymbol()
    {
        delete m_Symbols;
    }

    VariableSymbol* SymbolTable::InsertVariable(std::string name, ValueType dataType, VariableSymbolType variableType)
	{
        uint16_t index = GetLargestVariableIndex() + 1;
		return (VariableSymbol*)Insert(new VariableSymbol(name, dataType, index, variableType));
	}

	CallableSymbol* SymbolTable::InsertCallable(CallableSymbol callable)
	{
        return (CallableSymbol*)Insert(new CallableSymbol(callable));
	}

	ClassSymbol* SymbolTable::InsertClass(std::string name, ValueType dataType)
	{
		ClassSymbol* classSymbol = (ClassSymbol*)Insert(new ClassSymbol(name, dataType));
		classSymbol->m_Symbols = new SymbolTable(SymbolTableType::Local, nullptr);

		return classSymbol;
	}

    std::vector<Symbol*> SymbolTable::Lookup(std::function<bool(Symbol*)> predicate)
    {
        std::vector<Symbol*> result;
        LookupAccumulator(predicate, result);
        return result;
    }

    std::vector<Symbol*> SymbolTable::LookupThisTable(std::function<bool(Symbol*)> predicate)
    {
        std::vector<Symbol*> result;
        LookupThisTableAccumulator(predicate, result);
        return result;
    }

    std::vector<Symbol*> SymbolTable::Lookup(std::string name)
	{
		return Lookup([name](Symbol* symbol){
            return symbol->m_Name == name;
        });
	}

    std::vector<Symbol*> SymbolTable::Lookup(Symbol* symbol)
	{
        assert(symbol != nullptr);

        return Lookup([symbol](Symbol* s){
            return *s == *symbol;
        });
	}

	Symbol* SymbolTable::LookupOne(std::string name)
	{
		auto symbols = Lookup(name);
		if (symbols.empty())
			return nullptr;
		
		return symbols[0];
	}

	ClassSymbol* SymbolTable::LookupClassByType(ValueType type)
	{
		auto symbols = Lookup([type](Symbol* symbol){
            return symbol->m_DataType == type;
        });

        assert(symbols.size() <= 1);
        return (ClassSymbol*) One(symbols);
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


    Symbol* SymbolTable::Insert(Symbol* symbol)
	{
		// Make sure the *exact* symbol hasn't been declared before (uses the overloaded comparison operators on the symbols)
		assert(Lookup(symbol).empty());

        auto symbolsWithSameName = LookupThisTable([symbol](Symbol* s) {
            return s->m_Name == symbol->m_Name;
        });

        symbolsWithSameName.push_back(symbol);

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

        for (auto symbolsWithName : m_Symbols)
        {
            for (auto symbol : symbolsWithName)
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

        for (auto symbolsWithName : m_Symbols)
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

	SymbolTable::~SymbolTable()
	{
        for (auto symbolsWithName : m_Symbols)
        {
            for (auto symbol : symbolsWithName)
                delete symbol;
        }
	}

}
