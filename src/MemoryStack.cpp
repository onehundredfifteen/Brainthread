#include "MemoryStack.h"
#include "DebugLogStream.h"
#include "BrainThreadRuntimeException.h"

namespace BT {

	template <typename T>
	void MemoryStack<T>::Push(const T& n)
	{
		//if (mem_stack.size() > stack_limit)
		///	throw BFMemoryStackOverflowException();
		const std::lock_guard<std::mutex> lock(my_mutex);

		mem_stack.push(n);
	}

	//Funkcja zdejmuje i zwraca warto�� ze stosu. Gdy stos jest pusty, zwraca zero
	template <typename T>
	T MemoryStack<T>::Pop(void)
	{
		const std::lock_guard<std::mutex> lock(my_mutex);
		if (mem_stack.empty())
			return 0;

		T tmp;
		tmp = mem_stack.top();
		mem_stack.pop();

		return tmp;
	}

	//Swap two top elements of the stack. If there are less than 2 elements, do nothing.
	template <typename T>
	void MemoryStack<T>::Swap(void)
	{
		const std::lock_guard<std::mutex> lock(my_mutex);

		if (mem_stack.size() < 2)
			return;

		T tmp, tmp2;

		tmp = mem_stack.top();
		mem_stack.pop();

		tmp2 = mem_stack.top();
		mem_stack.pop();

		mem_stack.push(tmp);
		mem_stack.push(tmp2);
	}

	template < typename T >
	void MemoryStack<T>::PrintStack(std::ostream& s)
	{
		const std::lock_guard<std::mutex> lock(my_mutex);
		std::stack<T> st = mem_stack;

		s << "\n>Memory stack (fifo, " << st.size() << ")\n";
		while (!st.empty())
		{
			const std::lock_guard<std::mutex> lock(my_mutex);
			PrintCellValue<T>(s, st.top());
			st.pop();
			s << (st.empty() ? '\n' : ',');
		}
		s << std::flush;
	}

	// Explicit template instantiation
	template class MemoryStack<char>;
	template class MemoryStack<unsigned char>;
	template class MemoryStack<unsigned short>;
	template class MemoryStack<unsigned int>;
	template class MemoryStack<short>;
	template class MemoryStack<int>;
}