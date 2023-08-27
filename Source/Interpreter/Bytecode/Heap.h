#pragma once

#include <string>
#include <unordered_map>

namespace Ö
{

	constexpr int STACK_SIZE = 16;
	constexpr int STACKFRAME_SIZE = 16;

	class HeapEntry
	{
	public:
		HeapEntry() {};
		HeapEntry(int type, int id, char* data = nullptr) : m_Type(type), m_Id(id), m_Data(data) {};
		HeapEntry(int type, int id, const char* data = nullptr) : m_Type(type), m_Id(id), m_Data((char*)data) {};

		std::string ToString();

	public:
		int m_Type = 0;
		int m_Id = 0;

		void* m_Data = nullptr;
	};

	struct Heap
	{
		std::unordered_map<int, HeapEntry> m_Entries;

		int m_NextFreeId = 0;

		HeapEntry& CreateObject(int type, char* data = nullptr);

		HeapEntry& CreateString(const std::string& str = "");
		HeapEntry& CreateArray();
		HeapEntry& CreateObject();

		void DeleteObject(HeapEntry* obj);
		void DeleteObject(int id);
	};

}