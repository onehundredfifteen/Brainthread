#include "FunctionHeap.h"
#include "BrainThreadRuntimeException.h"

template < typename T >
FunctionHeap<T>::FunctionHeap(void)
{
	callingFunction = 0;
}

template < typename T >
FunctionHeap<T>::FunctionHeap(const FunctionHeap<T> &fun)
{
	callingFunction = 0;
	functions = fun.functions;
	//nie przepisujemy nic innego
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

//wywo�aj funkcje (przesu� wska�nik i wrzuc na stos wywo�anie)
template < typename T >
void FunctionHeap<T>::Call(T const& index, unsigned int * code_ptr)
{
	if( functions.find( index ) == functions.end()) 
		throw BFUndefinedFunctionException(index);  //funkcja nie istnieje

	  call_stack.push( std::make_pair(*code_ptr, index) );
	  *code_ptr = functions[index];

	  ++callingFunction;
}

//zako�cz wywo�anie - zrzu� ze stosu i powr�c do dawnej pozycji w kodzie
template < typename T >
void FunctionHeap<T>::Return(unsigned int * code_ptr)
{
	if(call_stack.empty() == false)
	{
		*code_ptr = call_stack.top().first;
		call_stack.pop();

		--callingFunction;
	}
}

//ile juz na stosie
template < typename T >
unsigned FunctionHeap<T>::Calls()
{
	return callingFunction;
}

//staktrace
template < typename T >
std::ostream& FunctionHeap<T>::PrintStackTrace(std::ostream &s)
{
  std::stack< std::pair< unsigned int, T > > call_stack_trace = call_stack; //wy�wietlenie go niszczy

  if( callingFunction )
  {
	  s << "\n>Stack trace (" << callingFunction << " " << ((callingFunction > 1)? "calls" : "call") << "):\n";
	  while(call_stack_trace.size() > 1)
	  {
		  s << ">\tat function #" << call_stack_trace.top().second << " (call from cell " << call_stack_trace.top().first << ")\n";

		  call_stack_trace.pop();
	  }
	  s << ">\tat function #" << call_stack_trace.top().second << " (call from cell " << call_stack_trace.top().first << ")";
  }
  else s << "\n>Stack trace: empty;" << std::endl;

  return s;
}

template < typename T >
std::ostream& FunctionHeap<T>::PrintDeclaredFunctions(std::ostream &s)
{
	std::map< T, unsigned int >::iterator mit;

	s << "\n>List of already defined functions ("<< functions.size() <<")";
	for(mit = functions.begin(); mit != functions.end(); ++mit)
	{
		if(std::is_signed<T>::value)
		   s << "\n>Id: " << static_cast<int>(mit->first);
		else
		   s << "\n>Id: " << static_cast<unsigned int>(mit->first);

		s << " Start point: " << mit->second;
	}

	return s << std::flush;
}

// Explicit template instantiation
template class FunctionHeap<char>;
template class FunctionHeap<unsigned char>;
template class FunctionHeap<unsigned short>;
template class FunctionHeap<unsigned int>;
template class FunctionHeap<short>;
template class FunctionHeap<int>;
