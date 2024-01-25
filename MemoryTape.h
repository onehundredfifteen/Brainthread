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

		std::ostream& SimpleMemoryDump(std::ostream& s, unsigned near_cells = 5);
		std::ostream& MemoryDump(std::ostream& o);

	protected:
		T* pointer; //piórko

		T* mem; //pamiec
		unsigned len; //aktualny rozmiar pamieci

		T* max_mem; //ostatnia komórka pamiêci

		const mem_option mem_behavior; //zachowanie pamieci
		const eof_option eof_behavior; //reakcja na EOF z wejœcia

		static const unsigned int double_mem_grow_limit = 2147483648; //2 Mb 
		//ten limit oznacza, ze do tej liczby objêtoœæ pamiêci siê dubluje,
		//potem dok³ada siê sta³a iloœci¹ - mem_grow_size

		static const unsigned int mem_grow_size = 104857600; //100 kb

		unsigned int GetNewMemorySize();
		void Realloc();
	};
}