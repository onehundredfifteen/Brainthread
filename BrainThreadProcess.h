#pragma once


#include "MemoryTape.h"
#include "MemoryHeap.h"
#include "FunctionHeap.h"
#include "CodeTape.h"
#include "ProcessMonitor.h"

//template < typename T >
//class BrainThread;

template < typename T >
class BrainThreadProcess
{
//template<typename T> friend class BrainThread;

public:
	BrainThreadProcess(ProcessMonitor * monitor, CodeTape * c, /*res_context r_ctx,*/ MemoryHeap<T> *shared_heap, unsigned int mem_size, typename MemoryTape<T>::mem_option mo, typename MemoryTape<T>::eof_option eo);
	BrainThreadProcess(const BrainThreadProcess<T> &parentProcess);
	~BrainThreadProcess(void);

	typedef enum {
		rcIndependent,
		rcShared
	} res_context;

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

	std::vector< HANDLE > child_threads;
	ProcessMonitor * process_monitor;
	res_context resource_context;

	void Fork(void);
	void Join(void);
};

