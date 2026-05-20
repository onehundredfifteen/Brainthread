#pragma once

#include <stack>
#include <mutex>
#include <ostream>

/*
 * Stack - stack to be used by language instructions. 
 It is shared between threads, so it must be thread-safe.
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

	private:
		mutable std::mutex my_mutex;

		static const unsigned int stack_limit = 65536;
	};
}