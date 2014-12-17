#pragma once

#include "MemoryTape.h"
#include "MemoryHeap.h"
#include "FunctionHeap.h"
#include "CodeTape.h"
#include "BFChildProcesses.h"

template < typename T >
class BrainThread;

template < typename T >
class BrainThreadProcess
{
template<typename T> friend class BrainThread;

public:
	BrainThreadProcess(CodeTape * c, MemoryHeap<T> *shared_heap, unsigned int mem_size, typename MemoryTape<T>::mem_option mo, typename MemoryTape<T>::eof_option eo);
	BrainThreadProcess(const BrainThreadProcess<T> &parentProcess);
	~BrainThreadProcess(void);

	void Run(void);

protected:
	MemoryTape<T> *memory;
	MemoryHeap<T> *heap;
	MemoryHeap<T> *shared_heap;

	FunctionHeap<T> *functions;

	CodeTape * code;
	std::stack<CodeTape::bt_instruction> jump_stack;
	CodeTape::bt_instruction current_instruction;

	unsigned int code_pointer;
	ChildProcesses<T> childs;

	void Fork();
};

