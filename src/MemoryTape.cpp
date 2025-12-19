#include <new>
#include <cstring> //memcpy, memset
#include <iostream>
#include <climits>

#include "MemoryTape.h"
#include "DebugLogStream.h"
#include "BrainThreadRuntimeException.h"

namespace BT {

	template < typename T >
	MemoryTape<T>::MemoryTape(unsigned int mem_size, eof_option eof_behavior, mem_option mem_behavior)
		: eof_behavior(eof_behavior), mem_behavior(mem_behavior)
	{
		try {
			mem = new T[mem_size];
		}
		catch (const std::bad_alloc&) {
			throw BFAllocException(mem_size, sizeof(T));
		}
		catch (...) {
			throw BFUnkownException();
		}

		len = mem_size;
		pointer = mem;
		max_mem = (T*)&mem[len - 1];

		std::memset(mem, 0, sizeof(T) * len);   //inicjujemy zerami
	}

	template < typename T >
	MemoryTape<T>::MemoryTape(const MemoryTape<T>& memory)
		: eof_behavior(memory.eof_behavior), mem_behavior(memory.mem_behavior)
	{
		try {
			mem = new T[memory.len];
		}
		catch (const std::bad_alloc&) {
			throw BFAllocException(memory.len, sizeof(T));
		}
		catch (...) {
			throw BFUnkownException();
		}

		len = memory.len;
		pointer = mem + memory.PointerPosition();
		max_mem = (T*)&mem[len - 1];

		memcpy(mem, memory.mem, sizeof(T) * len);
	}

	template < typename T >
	MemoryTape<T>::~MemoryTape(void)
	{
		delete[] mem;
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
	void MemoryTape<T>::Increment(int amount)
	{
		(*pointer) += amount;
	}
	template < typename T >
	void MemoryTape<T>::Decrement(void)
	{
		--(*pointer);
	}
	template < typename T >
	void MemoryTape<T>::Decrement(int amount)
	{
		(*pointer) -= amount;
	}

	template < typename T >
	void MemoryTape<T>::MoveRight(void)
	{
		if (pointer >= max_mem)
		{
			switch (mem_behavior)
			{
			case mem_option::moContinuousTape:
				pointer = mem; //na poczatek
				return;
			case mem_option::moDynamic:
			{
				try	{
					Realloc();
				}
				catch (const BrainThreadRuntimeException& e) {
					throw e;
				}
			}
			break;
			case mem_option::moLimited:
			default:
				throw BFRangeException(PointerPosition() + 1);
			}
		}
		++pointer;
	}

	template < typename T >
	void MemoryTape<T>::MoveRight(int amount)
	{
		do {
			MoveRight();
		} while (--amount);
	}

	template < typename T >
	void MemoryTape<T>::MoveLeft(void)
	{
		if (pointer <= mem)
		{
			switch (mem_behavior)
			{
			case mem_option::moContinuousTape:
				pointer = max_mem; //na koniec
				return;
			case mem_option::moDynamic:
			case mem_option::moLimited:
			default:
				throw BFRangeException(-1);
			}
		}
		--pointer;
	}

	template < typename T >
	void MemoryTape<T>::MoveLeft(int amount)
	{
		do {
			MoveLeft();
		} while (--amount);
	}

	template < typename T >
	void MemoryTape<T>::Read(void)
	{
		if (std::cin.peek() == std::char_traits<char>::eof())
		{
			switch (eof_behavior) {
				case eof_option::eoZero: *pointer = 0; return;
				case eof_option::eoMinusOne: *pointer = -1; return; 
			}
		}
		*pointer = std::cin.get();
	}
	/*
	funkcja ma dwie specjalizacje - ascii sa od 0 do 127
	Reszt� trzeba konwertowac na char
	*/
	template <>
	void MemoryTape<char>::Write(void)
	{
		std::cout << *pointer << std::flush;
	}
	template < typename T >
	void MemoryTape<T>::Write(void)
	{
		std::cout << static_cast<char>(*pointer) << std::flush;
	}

	template < typename T >
	void MemoryTape<T>::DecimalRead(void)
	{
		unsigned int i; //niewa�ne, czy signed czy unsigned
		std::cin >> i;

		if (std::cin.fail())
		{
			std::cin.clear();
			std::cin.ignore(UINT_MAX, '\n');
			throw BFInvalidInputStreamException();
		}
		*pointer = static_cast<T>(i);
	}

	template< typename T>
	void MemoryTape<T>::DecimalWrite(void)
	{
		if constexpr (std::is_signed<T>::value)
			std::cout << static_cast<int>(*pointer) << std::flush;
		else
			std::cout << static_cast<unsigned int>(*pointer) << std::flush;
	}

	/*Funkcje wewntrzne tasmy*/

	template < typename T >//funkcja zwraca nowa ilo�� pami�ci dla procesu
	unsigned int MemoryTape<T>::GetNewMemorySize()
	{
		return (len <= double_mem_grow_limit) ? 2 * len : len + mem_grow_size;
	}

	template < typename T > //realokuje pami�� (zmienia rozmiar pami�ci i kopiuje star� zawarto��)
	void MemoryTape<T>::Realloc()
	{
		T* new_mem;
		unsigned int new_mem_size = GetNewMemorySize();
		unsigned int p_pos = PointerPosition();

		try {
			new_mem = new T[new_mem_size];
		}
		catch (const std::bad_alloc&) {
			throw BFAllocException(new_mem_size, sizeof(T));
		}
		catch (...) {
			throw BFUnkownException();
		}

		//copy and zero the new chunk
		std::memset(new_mem + len, 0, sizeof(T) * (new_mem_size - len));
		std::memcpy(new_mem, mem, len);

		delete[] mem;

		mem = new_mem;
		pointer = mem + p_pos;
		len = new_mem_size;
		max_mem = (T*)&mem[len - 1];
	}

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

	template < typename T > //pokazuje n kom�rek w lewo i w prawo ze wska�nikiem mozliwie po�rodku
	void MemoryTape<T>::SimpleMemoryDump(std::ostream& s, unsigned near_cells)
	{
		unsigned int start = ((int)PointerPosition() - (int)near_cells) <= 0 ? 0 : (PointerPosition() - near_cells);
		const unsigned int end = near_cells * 2 + start;

		s << "\n>Memory Dump (cells " << start << "-" << end << ")\t";
		for (unsigned int i = start; i < len && i < end; ++i)
		{
			s << (PointerPosition() == i ? "<" : "") << i << (PointerPosition() == i ? ">" : ":");
			PrintCellValue<T>(s, mem[i]);
			s << " ";
		}
		s << std::endl;
	}

	template < typename T >
	void MemoryTape<T>::MemoryDump(std::ostream& o)
	{
		o << "\nBRAINTHREAD MEMORY DUMP (shows only nonzero cells)\n"
		  << "Pointer at: " << PointerPosition() << "\n"
		  << "Memory cell size [bytes]: " << sizeof(T) << "\n"
	      << "Memory size [cells], [bytes]: " << len << ", " << sizeof(T) * len << "\n"
		  << "Memory tape mode: ";

		switch (mem_behavior)
		{
			case mem_option::moContinuousTape: o << "continuous"; break;
			case mem_option::moDynamic: o << "dynamic"; break;
			case mem_option::moLimited:
			default: o << "limited";
		}

		unsigned int nz_cells = 0, last_nz = 0;
		for (unsigned int i = 0; i < len; ++i) {
			if (mem[i]) {
				++nz_cells;
				last_nz = i;
			}
		}

		o << "\nLast nonzero cell at: " << last_nz
		  << "\nNonzero cells count: " << nz_cells;

		o << "\n" << std::endl;

		for (unsigned int i = 0; i < len || i < last_nz; ++i)
		{
			if (mem[i])
			{
				o << "[" << i << "]";
				PrintCellValue<T>(o, mem[i]);
				
				o << ((--nz_cells % 10 == 0) ? '\n' : ' ');
			}
		}
		
		o << std::endl;
	}

	// Explicit template instantiation
	template class MemoryTape<char>;
	template class MemoryTape<unsigned char>;
	template class MemoryTape<unsigned short>;
	template class MemoryTape<unsigned int>;
	template class MemoryTape<short>;
	template class MemoryTape<int>;
}