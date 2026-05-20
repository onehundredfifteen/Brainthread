#pragma once

#include <list>
#include <thread>
#include <memory>

#include "MemoryTape.h"
#include "MemoryStack.h"
#include "FunctionStack.h"
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
		FunctionStack<T> functions;
		std::shared_ptr<MemoryStack<T>> stack;

		const CodeTape& code;
		unsigned int code_pointer;

		std::list<std::thread> child_threads;

		void Fork(void);
		void Join(void);
		void Detach(void);
		void ExecInstructions(void);

	private:
		mutable std::mutex _mutex;
		bool isMain;
	};
}

