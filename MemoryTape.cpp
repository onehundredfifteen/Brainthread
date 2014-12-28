#include <Windows.h>
#include <new>
#include <iostream>
#include <limits>

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
	pointer = nullptr;
	max_mem = nullptr;
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
			case MemoryTape::moContinuousTape:
				pointer = mem; //na poczatek
				return;
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
	if(pointer <= mem)
	{
		switch(mem_behavior)
		{
			case MemoryTape::moContinuousTape:
				pointer = max_mem; //na koniec
				return;
			case MemoryTape::moDynamic:
			case MemoryTape::moLimited:
			default:
				throw BFRangeException(-1);
		}
	}
	--pointer;
}

template < typename T >
void MemoryTape<T>::Read(void)
{
	EnterCriticalSection(&cout_critical_section);

	if(std::cin.peek() == std::char_traits<char>::eof())
	{
		 if(eof_behavior == MemoryTape<T>::eoZero)*pointer = 0;
		 else if(eof_behavior == MemoryTape<T>::eoMinusOne)*pointer = -1;
		 else //eoUnchanged
			 *pointer = std::cin.get();

	}
	else *pointer = std::cin.get();

	LeaveCriticalSection(&cout_critical_section);
}
/*
funkcja ma dwie specjalizacje - ascii sa od 0 do 127
Resztê trzeba konwertowac na char
}*/
template <>
void MemoryTape<char>::Write(void)
{
	EnterCriticalSection(&cout_critical_section);
	std::cout << *pointer << std::flush;
	LeaveCriticalSection(&cout_critical_section);
}
template < typename T >
void MemoryTape<T>::Write(void)
{
	EnterCriticalSection(&cout_critical_section);
	std::cout << static_cast<char>(*pointer) << std::flush;
	LeaveCriticalSection(&cout_critical_section);
}

template < typename T >
void MemoryTape<T>::DecimalRead(void)
{
	unsigned int i; //niewa¿ne, czy signed czy unsigned

	EnterCriticalSection(&cout_critical_section);
	std::cin >> i;

	if (std::cin.fail())
	{
		std::cin.clear();
		std::cin.ignore(UINT_MAX, '\n');
		LeaveCriticalSection(&cout_critical_section); //opuœæ sekcjê przed throw, aby nie zakleszczyæ w¹tków

		throw BFInvalidInputStreamException();
	}
	*pointer = static_cast<T>(i);

	LeaveCriticalSection(&cout_critical_section);
}

template< typename T>  
void MemoryTape<T>::DecimalWrite(void)
{
	EnterCriticalSection(&cout_critical_section);

	if(std::is_signed<T>::value)
		std::cout << static_cast<int>(*pointer) << std::flush;
	else
		std::cout << static_cast<unsigned int>(*pointer) << std::flush;

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

  //wszystko ok - to kopiujemy, now¹ pamiêæ zerujemy
  ZeroMemory( new_mem+len, new_mem_size-len );
  memcpy( new_mem, mem, len );
  

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

template < typename T >
inline T* const MemoryTape<T>::GetPointer() const
{
	return pointer;
}

template < typename T > //pokazuje n komórek w lewo i w prawo ze wskaŸnikiem mozliwie poœrodku
void MemoryTape<T>::SimpleMemoryDump(/*const ostream &o,*/ unsigned near_cells)
{
	unsigned int start = ((int)PointerPosition() - (int)near_cells) <= 0 ? 0 : (PointerPosition() - near_cells);

	for(unsigned int i = start; i < len && i < near_cells*2+start; ++i)
	{
		if(sizeof(T) == 1 || (sizeof(T) > 1 && mem[i] <= 255) ) 
		{
			if(std::is_signed<T>::value)
				std::cout << (PointerPosition() == i? '<' : '[') << i << (PointerPosition() == i? '>' : ']') << static_cast<char>(mem[i]) << ',' << static_cast<signed>(mem[i]) << ' ';
			else 
				std::cout << (PointerPosition() == i? '<' : '[') << i << (PointerPosition() == i? '>' : ']') << static_cast<char>(mem[i]) << ',' << static_cast<unsigned>(mem[i]) << ' ';
		}
		else
		{
			if(std::is_signed<T>::value)
				std::cout << (PointerPosition() == i? '<' : '[') << i << (PointerPosition() == i? '>' : ']') << static_cast<int>(mem[i]) << ' ';
			else 
				std::cout << (PointerPosition() == i? '<' : '[') << i << (PointerPosition() == i? '>' : ']') << static_cast<unsigned int>(mem[i]) << ' ';
		}
    }
	std::cout << std::endl;
}
/*
template < typename T > //pokazuje n niezerowych komórek
void MemoryTape<T>::MemoryDump(const ostream &o, unsigned n_nonzero_cells)
{
	for(unsigned int i = 0; i < len && i < n_nonzero_cells; ++i)
	{
	  if(mem[i])
		std::cout << '('<< i <<')'<< mem [i] << ' ';
    }
	std::cout << std::endl;
}*/


// Explicit template instantiation
template class MemoryTape<char>;
template class MemoryTape<unsigned char>;
template class MemoryTape<unsigned short>;
template class MemoryTape<unsigned int>;

