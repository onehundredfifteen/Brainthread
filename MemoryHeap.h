#pragma once

#include <stack>
#include <ostream>

/*
 * Klasa Stosu Pamiêci.
 * Pe³ni rolê pomocnicz¹ dla taœmy pamiêci. Dotatkowo pozwala zamieniaæ
 * ze sob¹ dwie ostatnie wartoœci. Wielkoœc stosu ogranicza zmienna 'stack_limit'.
*/

namespace BT {

	template < typename T >
	class MemoryHeap
	{
	public:
		MemoryHeap(void) {};

		void Push(const T&);
		T Pop(void);
		void Swap(void);

		void PrintStack(std::ostream& s);

	protected:
		std::stack<T> mem_stack;

		static const unsigned int stack_limit = 65536;
	};
}