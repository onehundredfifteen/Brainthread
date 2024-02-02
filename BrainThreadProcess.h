#pragma once

#include <list>
#include <thread>

#include "MemoryTape.h"
#include "MemoryHeap.h"
#include "FunctionHeap.h"
#include "CodeTape.h"

namespace BT {

	template < typename T >
	class BrainThreadProcess
	{
	public:
		BrainThreadProcess(const CodeTape& c, unsigned int mem_size, mem_option mo, eof_option eo);
		BrainThreadProcess(const BrainThreadProcess<T>& parentProcess);

		void Run(void);
		
		void PrintProcessInfo(std::ostream& s);

	private:
		MemoryTape<T> memory;
		MemoryHeap<T> heap;
		FunctionHeap<T> functions;
		
		std::shared_ptr<MemoryHeap<T>> shared_heap;
		const CodeTape& code;
		unsigned int code_pointer;

		std::list<std::thread> child_threads;

		void Fork(void);
		void Join(void);
		void ExecInstructions(void);

	private:
		bool isMain;
	};
}

