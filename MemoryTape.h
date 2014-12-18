#pragma once

#include <stack>

template < typename T >
class MemoryTape
{
	public:
		typedef enum 
		{
			moLimited,
			moDynamic,
			moTape
		} mem_option;

		typedef enum
		{
			eoZero,
			eoEOF,
			eoUnchanged
		} eof_option;

		MemoryTape(unsigned int mem_size, eof_option eof_behavior, mem_option option);
		MemoryTape(const MemoryTape<T> &memory);
		~MemoryTape(void);

		void Increment(void);
		void Decrement(void);

		void MoveLeft(void);
		void MoveRight(void);

		void Read(void);
		void Write(void);

		unsigned int PointerPosition() const;
		bool NullCell();
		void AfterFork(MemoryTape<T>* child_mem);

		void SimpleMemoryDump(unsigned n_nonzero_cells = 100);

	protected:
		T * pointer; //pi�rko

		T *mem; //pamiec
		unsigned len; //aktualny rozmiar pamieci

		T *max_mem; //ostatnia kom�rka pami�ci
		
		mem_option mem_behavior; //zachowanie pamieci
		eof_option eof_behavior; //reakcja na EOF z wej�cia

		static const unsigned int double_mem_grow_limit = 2147483648; //2 Mb 
		//ten limit oznacza, ze do tej liczby obj�to�� pami�ci si� dubluje,
		//potem dok�ada si� sta�a ilo�ci� - mem_grow_pack

		static const unsigned int mem_grow_pack = 104857600; //100 kb

		unsigned GetNewMemorySize();
		void Realloc();
};