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
		main_process = std::make_unique<BrainThreadProcess<T>>(tape, mem_size, mem_behavior, eof_behavior);
		main_process->Run();
	}

	// Explicit template instantiation
	template class Interpreter<char>;
	template class Interpreter<unsigned char>;
	template class Interpreter<unsigned short>;
	template class Interpreter<unsigned int>;
	template class Interpreter<short>;
	template class Interpreter<int>;
}