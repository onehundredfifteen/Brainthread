#include "FunctionHeap.h"
#include "DebugLogStream.h"
#include "BrainThreadRuntimeException.h"

namespace BT {

	template < typename T >
	FunctionHeap<T>::FunctionHeap() 
		: callingFunction(0)
	{
	}

	template < typename T >
	FunctionHeap<T>::FunctionHeap(const FunctionHeap<T>& fun)
	{
		callingFunction = 0;
		functions = fun.functions;
	}

	//add new function to list
	template < typename T >
	void FunctionHeap<T>::Add(T const& index, unsigned int const& code_ptr)
	{
		if (functions.find(index) != functions.end())
			throw BFExistantFunctionException(index);  //funkcja istnieje
		else if (call_stack.size() > stack_limit)
			throw BFFunctionStackOverflowException(); //stack overflow

		functions[index] = code_ptr + 1;
	}

	//call function (move code pointer to function body and put old position on call stack)
	template < typename T >
	void FunctionHeap<T>::Call(T const& index, unsigned int* code_ptr)
	{
		if (functions.find(index) == functions.end())
			throw BFUndefinedFunctionException(index);  //funkcja nie istnieje

		call_stack.push(std::make_pair(*code_ptr, index));
		*code_ptr = functions[index];

		++callingFunction;
	}

	//return a function - pop calling code position from call stack
	template < typename T >
	bool FunctionHeap<T>::Return(unsigned int* code_ptr)
	{
		if (call_stack.empty() == false)
		{
			*code_ptr = call_stack.top().first;
			call_stack.pop();

			--callingFunction;
			return true;
		}

		return false;
	}

	//call stack size
	template < typename T >
	unsigned FunctionHeap<T>::Calls() const
	{
		return callingFunction;
	}

	//stacktrace
	template < typename T >
	void FunctionHeap<T>::PrintStackTrace(std::ostream& s)
	{
		std::stack< std::pair< unsigned int, T > > call_stack_trace = call_stack; //wyœwietlenie go niszczy

		if (callingFunction)
		{
			s << "\n>Stack trace (" << callingFunction << " " << ((callingFunction > 1) ? "calls" : "call") << "):\n";
			while (call_stack_trace.size() > 1)
			{
				s << ">\tat function #" << call_stack_trace.top().second << " (call from cell " << call_stack_trace.top().first << ")\n";

				call_stack_trace.pop();
			}
			s << ">\tat function #" << call_stack_trace.top().second << " (call from cell " << call_stack_trace.top().first << ")";
		}
		else s << "\n>Stack trace: empty;" << std::endl;

		s << std::flush;
	}

	template < typename T >
	void FunctionHeap<T>::PrintDeclaredFunctions(std::ostream& s)
	{
		std::map< T, unsigned int >::iterator mit;

		s << "\n>List of already defined functions (" << functions.size() << ")";
		for (mit = functions.begin(); mit != functions.end(); ++mit)
		{
			s << "\n>Id: [";
			PrintCellValue<T>(s, mit->first);

			s << "] Start point: " << mit->second;
		}

		s << std::flush;
	}

	// Explicit template instantiation
	template class FunctionHeap<char>;
	template class FunctionHeap<unsigned char>;
	template class FunctionHeap<unsigned short>;
	template class FunctionHeap<unsigned int>;
	template class FunctionHeap<short>;
	template class FunctionHeap<int>;
}

