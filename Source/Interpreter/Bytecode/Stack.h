#pragma once

#include <vector>
#include <assert.h>

// Implementation of a stack that grows upwards and dynamicallly allocating more data if needed
template <class T>
class Stack
{
public:
	Stack<T>();
	Stack<T>(int startingSize);

	void Push(T value);
	T Pop();

	T& GetTop();
	bool HasMore();

	const std::vector<T>& GetData() { return m_Data; };

private:
	std::vector<T> m_Data;

	// The top variable points to the next free slot, so the slot above the top-most data
	int m_Top = 0; 
};


template <class T>
Stack<T>::Stack()
{
}

template <class T>
Stack<T>::Stack(int startingSize)
{
	m_Data.resize(startingSize);
}

template <class T>
void Stack<T>::Push(T value)
{
	assert(m_Top >= 0);

	// If there is available space in the stack
	if (!m_Data.empty() && m_Top < m_Data.size())
		m_Data[m_Top] = value;
	else
		m_Data.push_back(value); // Otherwise push to allocate more

	m_Top++;
}

template <class T>
T Stack<T>::Pop()
{
	// If nothing to pop, throw exception
	assert(m_Top >= 0 && m_Top <= m_Data.size());

	m_Top--;

	T top = m_Data[m_Top];

	m_Data[m_Top] = T();

	return top;
}

template <class T>
T& Stack<T>::GetTop()
{
	assert(m_Top > 0 && m_Top <= m_Data.size());

	return m_Data[m_Top - 1];
}

template<class T>
inline bool Stack<T>::HasMore()
{
	return m_Top > 0;
}
