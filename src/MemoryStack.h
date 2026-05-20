#pragma once

#include <stack>
#include <ostream>

/*
 * Klasa Stosu Pami�ci.
 * Pe�ni rol� pomocnicz� dla ta�my pami�ci. Dotatkowo pozwala zamienia�
 * ze sob� dwie ostatnie warto�ci. Wielko�c stosu ogranicza zmienna 'stack_limit'.
*/

namespace BT {

	template < typename T >
	class MemoryStack
	{
	public:
		MemoryStack(void) {};

		void Push(const T&);
		T Pop(void);
		void Swap(void);

		void PrintStack(std::ostream& s);

	protected:
		std::stack<T> mem_stack;

		static const unsigned int stack_limit = 65536;
	};
}