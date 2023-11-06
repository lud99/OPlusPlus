#include "Heap.h"

#include <map>

#include "../../Utils.hpp"

#include "../Value.h"

namespace Ö
{

	HeapEntry& Heap::CreateObject(int type, char* data)
	{
		int id = m_NextFreeId;
		m_Entries[id] = HeapEntry(0, id, data);

		m_NextFreeId++; // Increment to use a different slot for the next object

		return m_Entries[id];
	}

	HeapEntry& Heap::CreateString(const std::string& str)
	{
		int id = m_NextFreeId;
		m_Entries[id] = HeapEntry(0, id, (const char*)nullptr);

		char* strData = CopyString(str.c_str());

		m_Entries[id].m_Data = (void*)strData;

		m_NextFreeId++; // Increment to use a different slot for the next object

		return m_Entries[id];
	}

	HeapEntry& Heap::CreateArray()
	{
		int id = m_NextFreeId;
		m_Entries[id] = HeapEntry(1, id, (const char*)nullptr);

		ValueArray* arrayData = new ValueArray();
		m_Entries[id].m_Data = (void*)arrayData;

		m_NextFreeId++; // Increment to use a different slot for the next object

		return m_Entries[id];
	}

	HeapEntry& Heap::CreateObject()
	{
		int id = m_NextFreeId;
		m_Entries[id] = HeapEntry(2, id, (const char*)nullptr);

		ObjectInstance* objectData = new ObjectInstance();
		m_Entries[id].m_Data = (void*)objectData;

		m_NextFreeId++; // Increment to use a different slot for the next object

		return m_Entries[id];
	}

	void Heap::DeleteObject(HeapEntry* obj)
	{
		m_Entries.erase(obj->m_Id);

		obj->m_Data = nullptr;
	}

	void Heap::DeleteObject(int id)
	{
		delete m_Entries[id].m_Data;
		m_Entries[id].m_Data = nullptr;

		m_Entries.erase(id);
	}

	std::string HeapEntry::ToString()
	{
		return Value(*this).ToFormattedString(true);
	}
}