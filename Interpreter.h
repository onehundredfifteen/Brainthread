#pragma once

#include <list>
#include <windows.h>

#include "BrainThreadProcess.h"

namespace BT {

	class InterpreterBase {
	protected:
		const mem_option mem_behavior; //tape memory behavior 
		const eof_option eof_behavior; //input eof reaction setting
		const unsigned int mem_size;

	public:
		InterpreterBase(mem_option mem_behavior, eof_option eof_behavior, unsigned int mem_size)
			: mem_size(mem_size), mem_behavior(mem_behavior), eof_behavior(eof_behavior)
		{}

		virtual void Run(const CodeTape&) = 0;
	};
	
	template < typename T >
	class Interpreter: public InterpreterBase
	{	
	public:
		Interpreter(mem_option mem_behavior, eof_option eof_behavior, unsigned int mem_size);

		void Run(const CodeTape &);

	protected:
		MemoryHeap<T> shared_heap;
		std::unique_ptr<BrainThreadProcess<T>> main_process;
	};
}