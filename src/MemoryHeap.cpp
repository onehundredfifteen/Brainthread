#include "MemoryHeap.h"
#include "DebugLogStream.h"
#include "BrainThreadRuntimeException.h"

namespace BT {

	//Funkcja odk³ada wartoœæ na stos. Limit = stack_limit
	template <typename T>
	void MemoryHeap<T>::Push(const T& n)
	{
		if (mem_stack.size() > stack_limit)
			throw BFMemoryStackOverflowException();

		mem_stack.push(n);
	}

	//Funkcja zdejmuje i zwraca wartoœæ ze stosu. Gdy stos jest pusty, zwraca zero
	template <typename T>
	T MemoryHeap<T>::Pop(void)
	{
		if (mem_stack.empty())
			return 0;

		T tmp;
		tmp = mem_stack.top();
		mem_stack.pop();

		return tmp;
	}

	//Funkcja zamienia szczytowe dwie waroœci ze sob¹. 
	//Gdy stos ma mniej ni¿ 2 elementy, nic siê nie dzieje.
	template <typename T>
	void MemoryHeap<T>::Swap(void)
	{
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
	void MemoryHeap<T>::PrintStack(std::ostream& s)
	{
		std::stack<T> st = mem_stack;

		s << "\n>Memory stack (fifo, " << st.size() << ")\n";
		while (!st.empty())
		{
			PrintCellValue<T>(s, st.top());
			st.pop();
			s << (st.empty() ? '\n' : ',');
		}
		s << std::flush;
	}

	// Explicit template instantiation
	template class MemoryHeap<char>;
	template class MemoryHeap<unsigned char>;
	template class MemoryHeap<unsigned short>;
	template class MemoryHeap<unsigned int>;
	template class MemoryHeap<short>;
	template class MemoryHeap<int>;
}