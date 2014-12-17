#pragma once

#include <stack>

template < typename T >
class MemoryHeap
{
	public:
		MemoryHeap(void);
		~MemoryHeap(void);

		void Push(T const&);
		T Pop(void);
		void Swap(void);

	protected:
		std::stack<T> Stack;
		T tmp;
};