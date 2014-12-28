#pragma once

#include <stack>

/*
 * Klasa Stosu Pamiêci.
 * Pe³ni rolê pomocnicz¹ dla taœmy pamiêci. Dotatkowo pozwala zamieniaæ
 * ze sob¹ dwie ostatnie wartoœci. Wielkoœc stosu ogranicza zmienna 'stack_limit'.
*/

template < typename T >
class MemoryHeap
{
	public:
		MemoryHeap(void){};
		~MemoryHeap(void){};

		void Push(T const&);
		T Pop(void);
		void Swap(void);

	protected:
		std::stack<T> Stack;
		T tmp;

		static const unsigned int stack_limit = 65536;
};