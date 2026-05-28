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
		BrainThreadProcess(const CodeTape& c, const MemoryTape<T>& mem, const std::shared_ptr<MemoryStack<T>>& st);
		BrainThreadProcess(const BrainThreadProcess<T>& parentProcess);

		void Run(void);
		
		void PrintProcessInfo(std::ostream& s);

	private:
		const bool isMain;
		
		MemoryTape<T> memory;
		std::shared_ptr<MemoryStack<T>> stack;

		const CodeTape& code;
		unsigned int code_pointer;

		FunctionStack<T> functions;
		std::list<std::thread> child_threads;
		mutable std::mutex _mutex;

	private:
		void Fork(void);
		void Join(void);
		void Detach(void);
		void ExecInstructions(void);
	};
}

