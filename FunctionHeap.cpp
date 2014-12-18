//#include <iostream>

#include "FunctionHeap.h"
#include "BrainThreadRuntimeException.h"

template < typename T >
FunctionHeap<T>::FunctionHeap(void)
{
	callingFunction = 0;
}

template < typename T >
FunctionHeap<T>::~FunctionHeap(void)
{
}

//dodaj funkcje do listy
template < typename T >
void FunctionHeap<T>::Add(T const& index, unsigned int const& code_ptr)
{
	if( functions.find( index ) != functions.end()) 
		throw BFExistantFunctionException(index);  //funkcja istnieje
	else if( call_stack.size() > stack_limit) 
		throw BFFunctionStackOverflowException(); //stack overflow

    functions[index] = code_ptr + 1;
}

//wywo³aj funkcje (przesuñ wskaŸnik i wrzuc na stos wywo³anie)
template < typename T >
void FunctionHeap<T>::Call(T const& index, unsigned int * code_ptr)
{
	if( functions.find( index ) == functions.end()) 
		throw BFUndefinedFunctionException(index);  //funkcja nie istnieje

	  call_stack.push( std::make_pair(*code_ptr, index) );
	  *code_ptr = functions[index];

	  ++callingFunction;
}

//zakoñcz wywo³anie - zrzuæ ze stosu i powróc do dawnej pozycji w kodzie
template < typename T >
void FunctionHeap<T>::EndCall(unsigned int * code_ptr)
{
	*code_ptr = call_stack.top().first;
    call_stack.pop();

	--callingFunction;
}

//staktrace
template < typename T >
void FunctionHeap<T>::PrintStackTrace()
{
  std::ostringstream ss_text;
  std::stack< std::pair< unsigned int, T > > call_stack_trace = call_stack; //wyœwietlenie go niszczy

  ss_text.str( "" );
  if( callingFunction )
  {
	  ss_text << "\n> Stack trace ("<<callingFunction<<" "<< ((callingFunction > 1)? "calls" : "call") <<"):\n";
	  while(call_stack_trace.size() > 1)
	  {
		  ss_text << ">\tat function #" << call_stack_trace.top().second << " (call from cell "<< call_stack_trace.top().first<< ")\n";

		  call_stack_trace.pop();
	  }
	  ss_text << ">\tat function #" << call_stack_trace.top().second << " (call from cell "<< call_stack_trace.top().first<< ")";
  }
  else ss_text << "Stack trace: empty;\n";
}

// Explicit template instantiation
template class FunctionHeap<char>;
template class FunctionHeap<unsigned char>;
template class FunctionHeap<unsigned short>;
template class FunctionHeap<unsigned int>;
