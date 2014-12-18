#pragma once

#include <stack>
#include <map>

template < typename T >
class FunctionHeap
{
	public:
		FunctionHeap(void);
		~FunctionHeap(void);

		void Add(T const& index, unsigned int const& code_ptr);
		void Call(T const& index, unsigned int *code_ptr);
		void EndCall(unsigned int *code_ptr);

		void PrintStackTrace();

	protected:
		std::map< T, unsigned int > functions;
	    //indeks funkcji, wskaźnik na kod, pierwszy po nawiasie

	    std::stack< std::pair< unsigned int, T > > call_stack;
	    //stos funkcji - zapisujemy wskaźnik oraz id funkcji, podczas wywolania funkcji

	    unsigned callingFunction;
	    //teraz wywołujemy  przy okazji 'licznik stosu'

		static const unsigned int stack_limit = 65536;
};