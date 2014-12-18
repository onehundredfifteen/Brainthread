#include "MemoryHeap.h"

template <typename T>
MemoryHeap<T>::MemoryHeap(void)
{
}

template <typename T>
MemoryHeap<T>::~MemoryHeap(void)
{
}

template <typename T>
void MemoryHeap<T>::Push(T const& n)
{
	Stack.push(n);
}

template <typename T>
T MemoryHeap<T>::Pop(void)
{
	if(Stack.empty())
		return 0;
	
	tmp = Stack.top();
	Stack.pop();
	return tmp;
}

template <typename T>
void MemoryHeap<T>::Swap(void)
{
	tmp = Stack.top();
	Stack.pop();

	T tmp2 = Stack.top();
	Stack.pop();

	Stack.push(tmp);
	Stack.push(tmp2);
}

// Explicit template instantiation
template class MemoryHeap<char>;
template class MemoryHeap<unsigned char>;
template class MemoryHeap<unsigned short>;
template class MemoryHeap<unsigned int>;