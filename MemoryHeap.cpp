#include "MemoryHeap.h"
#include "BrainThreadRuntimeException.h"

/*
 * Klasa Stosu Pami�ci.
 * Pe�ni rol� pomocnicz� dla ta�my pami�ci. Dotatkowo pozwala zamienia�
 * ze sob� dwie ostatnie warto�ci. Wielko�c stosu ogranicza zmienna 'stack_limit'.
*/

//Funkcja odk�ada warto�� na stos. Limit = stack_limit
template <typename T>
void MemoryHeap<T>::Push(T const& n)
{
	if(Stack.size() > stack_limit)
		throw BFMemoryStackOverflowException();

	Stack.push(n);
}

//Funkcja zdejmuje i zwraca warto�� ze stosu. Gdy stos jest pusty, zwraca zero
template <typename T>
T MemoryHeap<T>::Pop(void)
{
	if(Stack.empty())
		return 0;
	
	tmp = Stack.top();
	Stack.pop();
	return tmp;
}

//Funkcja zamienia szczytowe dwie waro�ci ze sob�. 
//Gdy stos ma mniej ni� 2 elementy, nic si� nie dzieje.
template <typename T>
void MemoryHeap<T>::Swap(void)
{
	if(Stack.size() < 2)
		return;
	
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
template class MemoryHeap<short>;
template class MemoryHeap<int>;