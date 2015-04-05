#pragma once

#include <stack>
#include <map>
#include <ostream>

template < typename T >
class FunctionHeap
{
	public:
		FunctionHeap(void);
		FunctionHeap(const FunctionHeap<T> &fun);
		~FunctionHeap(void);

		void Add(T const& index, unsigned int const& code_ptr);
		void Call(T const& index, unsigned int *code_ptr);
		void Return(unsigned int *code_ptr);

		unsigned Calls(void);

		std::ostream& PrintStackTrace(std::ostream &s);
		std::ostream& PrintDeclaredFunctions(std::ostream &s);

	protected:
		std::map< T, unsigned int > functions;
	    //indeks funkcji, wskaźnik na kod, pierwszy po nawiasie

	    std::stack< std::pair< unsigned int, T > > call_stack;
	    //stos funkcji - zapisujemy wskaźnik oraz id funkcji, podczas wywolania funkcji

	    unsigned callingFunction;
	    //teraz wywołujemy  przy okazji 'licznik stosu'

		static const unsigned int stack_limit = 65536;
};