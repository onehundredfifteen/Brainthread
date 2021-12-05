#include <new>
#include <iostream>
#include <limits>
#include <windows.h>

#include "MemoryTape.h"
#include "ProcessMonitor.h"
#include "BrainThreadRuntimeException.h"

extern CRITICAL_SECTION cout_critical_section;

template < typename T >
MemoryTape<T>::MemoryTape(unsigned int mem_size, eof_option eof_behavior, mem_option mem_behavior)
{
  try
  {
    mem = new T[mem_size];
  }
  catch (std::bad_alloc ba)
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
  
  ZeroMemory( mem, sizeof(T) * len);   //inicjujemy zerami

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
  catch (std::bad_alloc ba)
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
  
  memcpy (mem, memory.mem, sizeof(T) * len);

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
	ProcessMonitor::EnterCriticalSection(cout_critical_section);

	if(std::cin.peek() == std::char_traits<char>::eof())
	{
		 if(eof_behavior == MemoryTape<T>::eoZero)*pointer = 0;
		 else if(eof_behavior == MemoryTape<T>::eoMinusOne)*pointer = -1;
		 else //eoUnchanged
			 *pointer = std::cin.get();

	}
	else *pointer = std::cin.get();

	ProcessMonitor::LeaveCriticalSection(cout_critical_section);
}
/*
funkcja ma dwie specjalizacje - ascii sa od 0 do 127
Resztê trzeba konwertowac na char
*/
template <>
void MemoryTape<char>::Write(void)
{
	ProcessMonitor::EnterCriticalSection(cout_critical_section);
	std::cout << *pointer << std::flush;
	ProcessMonitor::LeaveCriticalSection(cout_critical_section);
}
template < typename T >
void MemoryTape<T>::Write(void)
{
	ProcessMonitor::EnterCriticalSection(cout_critical_section);
	std::cout << static_cast<char>(*pointer) << std::flush;
	ProcessMonitor::LeaveCriticalSection(cout_critical_section);
}

template < typename T >
void MemoryTape<T>::DecimalRead(void)
{
	unsigned int i; //niewa¿ne, czy signed czy unsigned

	ProcessMonitor::EnterCriticalSection(cout_critical_section);
	std::cin >> i;

	if (std::cin.fail())
	{
		std::cin.clear();
		std::cin.ignore(UINT_MAX, '\n');
		ProcessMonitor::LeaveCriticalSection(cout_critical_section); //opuœæ sekcjê przed throw, aby nie zakleszczyæ w¹tków

		throw BFInvalidInputStreamException();
	}
	*pointer = static_cast<T>(i);

	ProcessMonitor::LeaveCriticalSection(cout_critical_section);
}

template< typename T>  
void MemoryTape<T>::DecimalWrite(void)
{
	ProcessMonitor::EnterCriticalSection(cout_critical_section);

	if(std::is_signed<T>::value) //!
		std::cout << static_cast<int>(*pointer) << std::flush;
	else
		std::cout << static_cast<unsigned int>(*pointer) << std::flush;

	ProcessMonitor::LeaveCriticalSection(cout_critical_section);
}

/*Funkcje wewntrzne tasmy*/

template < typename T >//funkcja zwraca nowa iloœæ pamiêci dla procesu
unsigned MemoryTape<T>::GetNewMemorySize()
{
	return (len <= double_mem_grow_limit) ? 2 * len : len + mem_grow_size;	
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
  catch (std::bad_alloc ba)
  {
	  throw BFAllocException(new_mem_size, sizeof(T));
  }
  catch(...)
  {
	  throw BFUnkownException();
  }

  //wszystko ok - to kopiujemy, now¹ pamiêæ zerujemy
  ZeroMemory( new_mem+len, sizeof(T) * (new_mem_size-len) );
  memcpy( new_mem, mem, len );
  

  //star¹ kasujemy
  delete [] mem;

  //ustawiamy co trzeba
  mem = new_mem;
  pointer = mem + p_pos;
  len = new_mem_size;
  max_mem = (T*) &mem[ len - 1 ];
}

//Zwraca pozycjê piórka
template < typename T >
inline unsigned int MemoryTape<T>::PointerPosition() const    
{
	return pointer - mem;
}

template < typename T >
inline T* const MemoryTape<T>::GetValue() const
{
	return pointer;
}
template < typename T >
inline void MemoryTape<T>::NullifyValue()
{
	*pointer = 0;
}

template < typename T > //pokazuje n komórek w lewo i w prawo ze wskaŸnikiem mozliwie poœrodku
std::ostream& MemoryTape<T>::SimpleMemoryDump(std::ostream &s, unsigned near_cells)
{
	unsigned int start = ((int)PointerPosition() - (int)near_cells) <= 0 ? 0 : (PointerPosition() - near_cells);

	s << "\n>Memory Dump (near " << near_cells <<" cells)";
	for(unsigned int i = start; i < len && i < near_cells*2+start; ++i)
	{
		if(sizeof(T) == 1 || (sizeof(T) > 1 && mem[i] <= 255) ) 
		{
			if(std::is_signed<T>::value)
				s << (PointerPosition() == i? "<" : "[") << i << (PointerPosition() == i? ">" : "]") << static_cast<char>(mem[i]) << "," << static_cast<signed>(mem[i]) << " ";
			else 
				s << (PointerPosition() == i? "<" : "[") << i << (PointerPosition() == i? ">" : "]") << static_cast<char>(mem[i]) << "," << static_cast<unsigned>(mem[i]) << " ";
		}
		else
		{
			if(std::is_signed<T>::value)
				s << (PointerPosition() == i? "<" : "[") << i << (PointerPosition() == i? ">" : "]") << static_cast<int>(mem[i]) << " ";
			else 
				s << (PointerPosition() == i? "<" : "[") << i << (PointerPosition() == i? ">" : "]") << static_cast<unsigned int>(mem[i]) << " ";
		}
    }

	return s << std::endl;
}

template < typename T > 
std::ostream& MemoryTape<T>::MemoryDump(std::ostream &o)
{
	unsigned int nz_cells = 0, last_nz = 0;
	
	o << "BRAINTHREAD MEMORY DUMP (shows only nonzero cells)\n"
	  << "Pointer at:" << PointerPosition() <<"\n"
	  << "Memory length [cells]: "<< len <<"\n"
	  << "Memory cell size [bytes]: "<< sizeof(T) << "\n"
	  << "Total memory used [bytes]: "<< sizeof(T) * len << "\n"
	  << "Memory tape mode: ";

	switch(mem_behavior)
	{
		case moContinuousTape: o << "continuous"; break;
		case moDynamic: o << "dynamic"; break;
		case moLimited:
		default: o << "limited"; 
	}

	o << "\n***" << std::endl;
	
	for(unsigned int i = 0; i < len; ++i)
	{
		if(mem[i] && (sizeof(T) == 1 || (sizeof(T) > 1 && mem[i] <= 255)) ) 
		{
			if(std::is_signed<T>::value)
				o << "[" << i <<  "]" << static_cast<char>(mem[i]) << "," << static_cast<signed>(mem[i]) << " ";
			else 
				o << "[" << i <<  "]" << static_cast<char>(mem[i]) << "," << static_cast<unsigned>(mem[i]) << " ";

			++nz_cells;
			last_nz = i;
		}
		else if(mem[i])
		{
			if(std::is_signed<T>::value)
				o << "[" << i <<  "]" << static_cast<int>(mem[i]) << " ";
			else 
				o << "[" << i <<  "]" << static_cast<unsigned int>(mem[i]) << " ";

			++nz_cells;
			last_nz = i;
			
		}	

		if(nz_cells%5 == 0) o << "\n";
    }

	o << "\nLast nonzero cell at: " << last_nz 
	  << "\nNonzero cells: " << nz_cells << "/"<< len;
	o << std::endl;

	return o;
}


// Explicit template instantiation
template class MemoryTape<char>;
template class MemoryTape<unsigned char>;
template class MemoryTape<unsigned short>;
template class MemoryTape<unsigned int>;
template class MemoryTape<short>;
template class MemoryTape<int>;

