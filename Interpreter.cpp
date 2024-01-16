#include <iostream>


#include "Interpreter.h"
#include "BrainThreadRuntimeException.h"

namespace BT {

	template < typename T >
	Interpreter<T>::Interpreter(mem_option mem_behavior, eof_option eof_behavior, unsigned int mem_size)
		: InterpreterBase(mem_behavior, eof_behavior, mem_size)
	{
	}

	template < typename T >
	void Interpreter<T>::Run(const CodeTape& tape)
	{
		try
		{
			main_process = std::make_unique<BrainThreadProcess<T>>(tape, shared_heap, mem_size, mem_behavior, eof_behavior);
			main_process->Run();
		}
		catch (const BrainThreadRuntimeException& re)
		{
			std::cerr << "<main> " << re.what() << std::endl;
		}
		catch (const std::exception& e)
		{
			std::cerr << "<main> " << e.what() << std::endl;
		}
		catch (...)
		{
			std::cerr << "<main> FATAL ERROR" << std::endl;
		}
	}

	// Explicit template instantiation
	template class Interpreter<char>;
	template class Interpreter<unsigned char>;
	template class Interpreter<unsigned short>;
	template class Interpreter<unsigned int>;
	template class Interpreter<short>;
	template class Interpreter<int>;
}