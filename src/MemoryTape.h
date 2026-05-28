#pragma once

#include <stack>
#include <ostream>

#include "Enumdefs.h"

namespace BT {

	template < typename T >
	class MemoryTape
	{
	public:		
		MemoryTape(unsigned int mem_size, eof_option eof_behavior, mem_option option);
		MemoryTape(const MemoryTape<T>& memory);
		~MemoryTape(void);

		void Increment(void);
		void Increment(int);
		void Decrement(void);
		void Decrement(int);

		void MoveLeft(void);
		void MoveLeft(int);
		void MoveRight(void);
		void MoveRight(int);

		void Read(void);
		void Write(void);
		void DecimalRead(void);
		void DecimalWrite(void);

		unsigned int GetPointerPosition() const;
		T* const GetValue() const;

		void SimpleMemoryDump(std::ostream& s, unsigned near_cells = 5) const;
		void MemoryDump(std::ostream& o) const;

	private:
		T* pointer; 

		T* mem; //memory (start)
		T* max_mem; //last cell
		unsigned len; 

		const mem_option mem_behavior;
		const eof_option eof_behavior;

		static const unsigned int double_mem_grow_limit = 2147483648; //2 Mb 
		static const unsigned int mem_grow_size = 104857600; //100 kb

		unsigned int findNewMemorySize();
		void realloc();
	};
}