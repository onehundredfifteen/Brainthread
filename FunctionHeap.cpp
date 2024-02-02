#include "FunctionHeap.h"
#include "DebugLogStream.h"
#include "BrainThreadRuntimeException.h"

namespace BT {

	template < typename T >
	FunctionHeap<T>::FunctionHeap() 
	{
	}

	template < typename T >
	FunctionHeap<T>::FunctionHeap(const FunctionHeap<T>& fun)
	{
		functions = fun.functions;
	}

	//add new function to list
	template < typename T >
	void FunctionHeap<T>::Add(T const& index, unsigned int const& code_ptr)
	{
		if (functions.find(index) != functions.end())
			throw BFExistantFunctionException(index);  //funkcja istnieje
	
		functions[index] = code_ptr + 1;
	}

	//call function (move code pointer to function body and put old position on call stack)
	template < typename T >
	void FunctionHeap<T>::Call(T const& index, unsigned int* code_ptr)
	{
		if (functions.find(index) == functions.end())
			throw BFUndefinedFunctionException(index);  //funkcja nie istnieje
		else if (call_stack.size() > stack_limit)
			throw BFFunctionStackOverflowException(); //stack overflow

		call_stack.push(std::make_pair(*code_ptr, index));
		*code_ptr = functions[index];
	}

	//return a function - pop calling code position from call stack
	template < typename T >
	bool FunctionHeap<T>::Return(unsigned int* code_ptr)
	{
		if (call_stack.empty() == false)
		{
			*code_ptr = call_stack.top().first;
			call_stack.pop();
			return true;
		}
		return false;
	}

	//call stack size
	template < typename T >
	unsigned FunctionHeap<T>::Calls() const
	{
		return call_stack.size();
	}

	//stacktrace
	template < typename T >
	void FunctionHeap<T>::PrintStackTrace(std::ostream& s)
	{
		std::stack<std::pair<unsigned int, T>> call_stack_trace = call_stack;

		s << "\n>Stack trace (" << Calls() << " " << ((Calls() > 1) ? "calls" : "call") << "):\n";
		while (!call_stack_trace.empty())
		{
			s << ">\tat function #";
			PrintCellValue<T>(s, call_stack_trace.top().second);
			s << " (call from position " << call_stack_trace.top().first << ")\n";
			call_stack_trace.pop();
		}

		s << std::flush;
	}

	template < typename T >
	void FunctionHeap<T>::PrintDeclaredFunctions(std::ostream& s)
	{
		s << "\n>List of already defined functions (" << functions.size() << ")";
		for (std::map<T, unsigned int>::const_iterator mit = functions.begin(); mit != functions.end(); ++mit)
		{
			s << "\n>Id: [";
			PrintCellValue<T>(s, mit->first);
			s << "] Start point: " << mit->second;
		}
		s << std::endl;
	}

	// Explicit template instantiation
	template class FunctionHeap<char>;
	template class FunctionHeap<unsigned char>;
	template class FunctionHeap<unsigned short>;
	template class FunctionHeap<unsigned int>;
	template class FunctionHeap<short>;
	template class FunctionHeap<int>;
}

