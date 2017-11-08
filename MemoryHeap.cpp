#include "MemoryHeap.h"
#include "BrainThreadRuntimeException.h"

/*
 * Klasa Stosu Pamiêci.
 * Pe³ni rolê pomocnicz¹ dla taœmy pamiêci. Dotatkowo pozwala zamieniaæ
 * ze sob¹ dwie ostatnie wartoœci. Wielkoœc stosu ogranicza zmienna 'stack_limit'.
*/

//Funkcja odk³ada wartoœæ na stos. Limit = stack_limit
template <typename T>
void MemoryHeap<T>::Push(T const& n)
{
	if(mem_stack.size() > stack_limit)
		throw BFMemoryStackOverflowException();

	mem_stack.push(n);
}

//Funkcja zdejmuje i zwraca wartoœæ ze stosu. Gdy stos jest pusty, zwraca zero
template <typename T>
T MemoryHeap<T>::Pop(void)
{
	if(mem_stack.empty())
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
	if(mem_stack.size() < 2)
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
std::ostream& MemoryHeap<T>::PrintStack(std::ostream &s)
{
	std::stack<T> st = mem_stack; //wyœwietlenie go niszczy

	s << "\n>Memory stack ("<< st.size() <<")\n";
	while(true)
	{
		if(std::is_signed<T>::value)
		   s << static_cast<int>(st.top());
		else
		   s << static_cast<unsigned int>(st.top());

		if(st.empty() == false)
		{
		   s << ", ";
		   st.pop();
		}
		else
		{
		   s << ";";
		   break;
		}
	}

	return s;
}

// Explicit template instantiation
template class MemoryHeap<char>;
template class MemoryHeap<unsigned char>;
template class MemoryHeap<unsigned short>;
template class MemoryHeap<unsigned int>;
template class MemoryHeap<short>;
template class MemoryHeap<int>;