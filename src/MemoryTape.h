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

		unsigned int PointerPosition() const;
		T* const GetValue() const;

		void SimpleMemoryDump(std::ostream& s, unsigned near_cells = 5);
		void MemoryDump(std::ostream& o);

	protected:
		T* pointer; 

		T* mem; //memory
		unsigned len; 

		T* max_mem; //last cell

		const mem_option mem_behavior; //zachowanie pamieci
		const eof_option eof_behavior; //reakcja na EOF z wej�cia

		static const unsigned int double_mem_grow_limit = 2147483648; //2 Mb 
		//ten limit oznacza, ze do tej liczby obj�to�� pami�ci si� dubluje,
		//potem dok�ada si� sta�a ilo�ci� - mem_grow_size

		static const unsigned int mem_grow_size = 104857600; //100 kb

		unsigned int GetNewMemorySize();
		void Realloc();
	};
}