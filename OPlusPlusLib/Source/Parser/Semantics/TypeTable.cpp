#include "TypeTable.h"

#include <assert.h>

namespace O
{
	TypeTableEntry& TypeTableEntry::Resolve()
	{
		TypeTableEntry* currentEntry = this;
		while (currentEntry->type == TypeTableType::Typedef)
		{
			assert(currentEntry->redirect != nullptr);

			currentEntry = currentEntry->redirect;
		}

		return *currentEntry;
	}

	TypeTable::TypeTable()
	{
		// Add the build in primitives to the typeEntry table 
		const std::string builtInTypes[] = { "void", "int", "float", "string" };

		for (uint16_t i = 0; i < 4; i++)
			m_Types[builtInTypes[i]] = { builtInTypes[i], i };

		m_CurrentFreeTypeId = m_Types.size();
	}

	TypeTableEntry& TypeTable::GetEntryFromId(ValueType id)
	{
		for (auto& entry : m_Types)
		{
			if (entry.second.id == id)
				return entry.second;
		}

		abort();
	}

	TypeTableEntry& TypeTable::Add(const std::string& typeName, TypeTableType type, TypeTableEntry* redirect)
	{
		assert (!HasType(typeName));

		m_Types[typeName] = { typeName, m_CurrentFreeTypeId++, type, redirect };
		return m_Types[typeName];
	}
	TypeTableEntry& TypeTable::AddPrivateType(const std::string& typeName, TypeTableType type, TypeTableEntry* redirect)
	{
		assert (!HasType(typeName));

		m_Types[typeName] = { typeName, m_CurrentFreeTypeId++, type, redirect, true };
		return m_Types[typeName];
	}
	TypeTableEntry& TypeTable::ResolveEntry(TypeTableEntry entry)
	{
		return entry.Resolve();
	}
}