#pragma once

#include <windows.h>

#include "MemoryTape.h"
#include "MemoryHeap.h"
#include "FunctionHeap.h"
#include "CodeTape.h"

namespace BT {

	template < typename T >
	class BrainThreadProcess
	{
	public:
		BrainThreadProcess(const CodeTape& c, MemoryHeap<T>& shared_heap, unsigned int mem_size, mem_option mo, eof_option eo);
		BrainThreadProcess(const BrainThreadProcess<T>& parentProcess);
		~BrainThreadProcess(void);

		void Run(void);

		unsigned int GetProcessId(void);
		std::ostream& PrintProcessInfo(std::ostream& s);

	protected:
		MemoryTape<T> memory;
		MemoryHeap<T> heap;
		FunctionHeap<T> functions;
		MemoryHeap<T>& shared_heap;

		const CodeTape& code;
		unsigned int code_pointer;

		std::vector< HANDLE > child_threads;

		void Fork(void);
		void Join(void);

	private:
		bool isMain;
	};
}

