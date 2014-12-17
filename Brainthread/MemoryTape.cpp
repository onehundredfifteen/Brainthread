#include <Windows.h>
#include <new>
#include <iostream>

#include "MemoryTape.h"
#include "BrainThreadRuntimeException.h"

extern CRITICAL_SECTION cout_critical_section;

template < typename T >
MemoryTape<T>::MemoryTape(unsigned int mem_size, eof_option eof_behavior, mem_option mem_behavior)
{
  try
  {
    mem = new T[mem_size];
  }
  catch (std::bad_alloc& ba)
  {
    throw BFAllocException(mem_size, sizeof(T));
  }
  catch(...)
  {
	throw BFUnkownException();
  }
  
  len = mem_size;
  pointer = mem;
  max_mem = (T*) &mem[  len - 1 ];
  
  ZeroMemory( mem, len );   //inicjujemy zerami

  this->eof_behavior = eof_behavior;
  this->mem_behavior = mem_behavior;
}

template < typename T >
MemoryTape<T>::MemoryTape(const MemoryTape<T> &memory)
{
  try
  {
    mem = new T[memory.len];
  }
  catch (std::bad_alloc& ba)
  {
    throw BFAllocException(memory.len, sizeof(T));
  }
  catch(...)
  {
	throw BFUnkownException();
  }
  
  len = memory.len;
  pointer = mem + memory.PointerPosition();

  max_mem = (T*) &mem[ len - 1 ];
  
  memcpy (mem, memory.mem, len);

  eof_behavior = memory.eof_behavior;
  mem_behavior = memory.mem_behavior;
}

template < typename T >
MemoryTape<T>::~MemoryTape(void)
{
	delete [] mem;
	pointer = NULL;
	max_mem = NULL;
	len = 0;
}

/*Funkcje  - komendy*/
template < typename T >
void MemoryTape<T>::Increment(void)
{
	 ++(*pointer);
}
template < typename T >
void MemoryTape<T>::Decrement(void)
{
	--(*pointer);
}

template < typename T >
void MemoryTape<T>::MoveRight(void)
{
	if(pointer >= max_mem)
	{
		switch(mem_behavior)
		{
			case MemoryTape::moTape:
				pointer = mem; //na poczatek
				break;
			case MemoryTape::moDynamic:
				{
					try
					{
						Realloc();
					}
					catch(BrainThreadRuntimeException &e)
					{
						throw e;
					}
				}
				break;
			case MemoryTape::moLimited:
			default:
				throw BFRangeException(PointerPosition()+1);
		}
	}
	++pointer;
}

template < typename T >
void MemoryTape<T>::MoveLeft(void)
{
	if(pointer < mem)
	{
		switch(mem_behavior)
		{
			case MemoryTape::moTape:
				pointer = max_mem; //na koniec
				break;
			case MemoryTape::moDynamic:
			case MemoryTape::moLimited:
			default:
				throw BFRangeException(PointerPosition()-1);
		}
	}
	--pointer;
}

template < typename T >
void MemoryTape<T>::Read(void)
{
	if(std::cin.peek() == std::char_traits<char>::eof())
	{
		 if(eof_behavior == MemoryTape<T>::eoZero)*pointer = 0;
		 else if(eof_behavior == MemoryTape<T>::eoEOF)*pointer = -1;
		 else //eoUnchanged
			 *pointer = std::cin.get();

	}
	else *pointer = std::cin.get();
}

template < typename T >
void MemoryTape<T>::Write(void)
{
	EnterCriticalSection(&cout_critical_section);
	std::cout << *pointer << std::flush;
	LeaveCriticalSection(&cout_critical_section);
}

/*Funkcje wewntrzne tasmy*/

template < typename T >//funkcja zwraca nowa iloœæ pamiêci dla procesu
unsigned MemoryTape<T>::GetNewMemorySize()
{
	return (len <= double_mem_grow_limit) ? 2 * len : len + mem_grow_pack;	
}


template < typename T > //realokuje pamiêæ (zmienia rozmiar pamiêci i kopiuje star¹ zawartoœæ)
void MemoryTape<T>::Realloc()
{
  T *new_mem;
  unsigned int new_mem_size = GetNewMemorySize();
  unsigned int p_pos = PointerPosition();

  try
  {
      new_mem = new T[new_mem_size];
  }
  catch (std::bad_alloc& ba)
  {
	  throw BFAllocException(new_mem_size, sizeof(T));
  }
  catch(...)
  {
	  throw BFUnkownException();
  }

  //wszystko ok - to kopiujemy
  memcpy( new_mem, mem, sizeof(mem) );

  //star¹ kasujemy
  delete [] mem;

  //ustawiamy co trzeba
  mem = new_mem;
  pointer = mem + p_pos;
  len = new_mem_size;
  max_mem = (T*) &mem[  len - 1 ];
}

//Zwraca pozycjê piórka
template < typename T >
inline unsigned int MemoryTape<T>::PointerPosition()  const    
{
	return pointer - mem;
}

//Mówi, czy aktualna komórka pamiêci jest wyzerowana, czy nie.
template < typename T >
inline bool MemoryTape<T>::NullCell()
{
	return *pointer == 0;
}

template < typename T > //pokazuje n niezerowych komórek
void MemoryTape<T>::SimpleMemoryDump(unsigned n_nonzero_cells)
{
	for(unsigned int i = 0; i < len && i < n_nonzero_cells; ++i)
	{
	  if(mem[i])
		std::cout << '('<< i <<')'<< mem [i] << ' ';
    }
	std::cout << std::endl;
}


// Explicit template instantiation
template class MemoryTape<char>;
template class MemoryTape<unsigned char>;
template class MemoryTape<unsigned short>;
template class MemoryTape<unsigned int>;