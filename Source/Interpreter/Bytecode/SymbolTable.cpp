#include "SymbolTable.h"

#include <assert.h>

namespace Ö
{
	TypeTableEntry& SymbolTable::Symbol::GetTypeTableEntry()
	{
		return m_StorableValueType->Resolve();
	}

	SymbolTable::Symbol::Symbol(std::string name, SymbolType symbolType, TypeTableEntry* storableValueType)
	{
		m_Name = name;
		m_SymbolType = symbolType;
		m_StorableValueType = storableValueType;
	}

	SymbolTable::VariableSymbol::VariableSymbol(std::string name, SymbolType symbolType, TypeTableEntry* storableValueType, uint16_t index)
	{
		m_Name = name;
		m_SymbolType = symbolType;
		m_StorableValueType = storableValueType;
		
		m_Index = index;
	}

	/*std::size_t SymbolTable::Symbol::operator()(const Symbol& symbol) const noexcept
	{
		return m_Id;
	}*/

	SymbolTable::VariableSymbol* SymbolTable::InsertVariable(int scope, std::string name, TypeTableEntry* storeType)
	{
		return (VariableSymbol*)Insert(scope, name, storeType, SymbolType::Variable);
	}

	SymbolTable::FunctionSymbol* SymbolTable::InsertFunction(int scope, std::string name, TypeTableEntry* returnType, uint16_t id)
	{
		return (FunctionSymbol*)Insert(scope, name, returnType, SymbolType::Function, id);
	}

	SymbolTable::FunctionSymbol* SymbolTable::InsertMethod(int scope, std::string name, TypeTableEntry* returnType, uint16_t id)
	{
		return (FunctionSymbol*)Insert(scope, name, returnType, SymbolType::Method, id);
	}

	SymbolTable::ClassSymbol* SymbolTable::InsertClass(int scope, std::string name, TypeTableEntry* valueType)
	{
		ClassSymbol* classSymbol = (ClassSymbol*)Insert(scope, name, valueType, SymbolType::Class);
		classSymbol->m_MemberVariables = new SymbolTable(nullptr, scope);
		classSymbol->m_Methods = new SymbolTable(nullptr, scope);

		return classSymbol;
	}

	SymbolTable::Symbol* SymbolTable::Insert(int scope, std::string name, TypeTableEntry* valueType, SymbolType symbolType, uint16_t id)
	{
		// Make sure the symbol hasn't been declared before
		assert(Lookup(name) == nullptr);

		// If not at the scope that the variable should be created in
		if (m_Scope != scope)
		{
			// Traverse the symbol tables and create them if necessary, until the desired scope is reached.
			SymbolTable* table = this;
			while (table->m_Scope < scope)
			{
				if (!table->m_NextSymbolTable)
					table->m_NextSymbolTable = new SymbolTable(this, table->m_Scope + 1);

				table = table->m_NextSymbolTable;
			}

			// Insert at the correct table
			return table->Insert(scope, name, valueType, symbolType, id);
		}

		if (symbolType == SymbolType::Variable)
		{
			// Find the variable with the highest index at all scopes.
			// That + 1 is the index slot the variable will occupy.
			uint16_t variableIndex = GetLargestVariableIndex() + 1;

			m_Symbols[name] = new VariableSymbol(name, symbolType, valueType, variableIndex);
		}
		else if (symbolType == SymbolType::Function)
		{
			auto symbol = new FunctionSymbol(name, symbolType, valueType);
			symbol->m_Id = id;

			m_Symbols[name] = symbol;
		}
		else if (symbolType == SymbolType::Method)
		{
			auto symbol = new FunctionSymbol(name, symbolType, valueType);
			symbol->m_Id = id; // GetLargestMethodIndex() + 1;

			m_Symbols[name] = symbol;
		}
		else if (symbolType == SymbolType::Class)
			m_Symbols[name] = new ClassSymbol(name, symbolType, valueType);
		else
			m_Symbols[name] = new Symbol{ name, symbolType, valueType };

		return m_Symbols[name];
	}

	SymbolTable::Symbol* SymbolTable::Lookup(std::string name)
	{
		// First search in the current symbol table
		if (m_Symbols.count(name) != 0)
			return m_Symbols[name];

		// Otherwise, resursively search the nested symbol tables
		SymbolTable* table = this;
		while (table->m_NextSymbolTable != nullptr)
		{
			table = table->m_NextSymbolTable;
			Symbol* symbol = table->Lookup(name);
			if (symbol != nullptr)
				return symbol;

			//table = table->m_NextSymbolTable;
		}

		return nullptr;
	}

	SymbolTable::ClassSymbol* SymbolTable::LookupClassByType(ValueType type)
	{
		// First search in the current symbol table
		for (auto& entry : m_Symbols)
		{
			if (entry.second->m_StorableValueType->Resolve().id == type && entry.second->m_SymbolType == SymbolType::Class)
				return (SymbolTable::ClassSymbol*)entry.second;
		}
			
		// Otherwise, resursively search the nested symbol tables
		SymbolTable* table = this;
		while (table->m_NextSymbolTable != nullptr)
		{
			table = table->m_NextSymbolTable;
			Symbol* symbol = table->LookupClassByType(type);
			if (symbol != nullptr)
				return (SymbolTable::ClassSymbol*)symbol;

			//table = table->m_NextSymbolTable;
		}

		return nullptr;
	}

	void SymbolTable::Remove(int scopeToRemove)
	{
		assert(scopeToRemove != 0);

		if (scopeToRemove - 1 > m_Scope)
		{
			return m_NextSymbolTable->Remove(scopeToRemove);
		}
		else
		{
			delete m_NextSymbolTable;
			m_NextSymbolTable = nullptr;
		}
	}

	// Recursively find the largest variable index in the symbol table above
	int SymbolTable::GetLargestVariableIndex()
	{
		int largestIndex = -1;
		if (m_PreviousSymbolTable) 
			largestIndex = m_PreviousSymbolTable->GetLargestVariableIndex();

		for (auto& symbol : m_Symbols)
		{
			if (symbol.second->m_SymbolType != SymbolType::Variable)
				continue;

			SymbolTable::VariableSymbol* variableSymbol = (SymbolTable::VariableSymbol*)symbol.second;

			if (variableSymbol->m_Index > largestIndex)
				largestIndex = variableSymbol->m_Index;
		}

		return largestIndex;
	}

	int SymbolTable::GetLargestMethodIndex()
	{
		int largestIndex = -1;
		//if (m_PreviousSymbolTable)
			//largestIndex = m_PreviousSymbolTable->GetLargestMethodIndex();

		for (auto& symbol : m_Symbols)
		{
			if (symbol.second->m_SymbolType != SymbolType::Method)
				continue;

			SymbolTable::FunctionSymbol* methodSymbol = (SymbolTable::FunctionSymbol*)symbol.second;

			if (methodSymbol->m_Id > largestIndex)
				largestIndex = methodSymbol->m_Id;
		}

		return largestIndex;
	}

	bool SymbolTable::Has(std::string name)
	{
		return Lookup(name) != nullptr;
	}

	bool SymbolTable::HasAndIs(std::string name, SymbolType type)
	{
		if (!Has(name)) return false;

		return Lookup(name)->m_SymbolType == type;
	}

	SymbolTable::~SymbolTable()
	{
		for (auto& entry : m_Symbols)
		{
			if (entry.second->m_SymbolType == SymbolType::Class)
			{
				SymbolTable::ClassSymbol* s = (SymbolTable::ClassSymbol*)(entry.second);
				delete s->m_Methods;
				delete s->m_MemberVariables;
			}

			delete entry.second;
		}

		delete m_NextSymbolTable;
	}
}
