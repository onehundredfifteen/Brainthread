#include <Windows.h>
#include <process.h>
#include "BrainThreadProcess.h"
#include "BrainThreadRuntimeException.h"

extern CRITICAL_SECTION critical_section;

#include <iostream>

template < typename T >
void __cdecl  run_bt_thread(void * arg) 
{
	try
	{
		((BrainThreadProcess<T>*)arg)->Run();
		_endthread();
	}
	catch(BrainThreadRuntimeException &re)
	{
		std::cout << "ThreadId:" <<  GetCurrentThreadId() << "> "<< re.what() << std::endl;
	}
	catch(std::exception &e)
	{
		std::cout << "ThreadId:" <<  GetCurrentThreadId() << "> "<< e.what() << std::endl;
	}
	catch(...)
	{
		std::cout << "ThreadId:" <<  GetCurrentThreadId() << "> FATAL ERROR" << std::endl;
	}
}

template < typename T >
BrainThreadProcess<T>::BrainThreadProcess(CodeTape * c, MemoryHeap<T> *shared_heap, unsigned int mem_size, typename MemoryTape<T>::mem_option mo, typename MemoryTape<T>::eof_option eo)
{
	 code = c;
	 memory = new MemoryTape<T>(mem_size, eo, mo);
	 this->shared_heap = shared_heap;

	 code_pointer = 0;
}

template < typename T >
BrainThreadProcess<T>::BrainThreadProcess(const BrainThreadProcess<T> &parentProcess)
{
	code = parentProcess.code;
	memory = new MemoryTape<T>(*parentProcess.memory);
	shared_heap = parentProcess.shared_heap;
	functions = parentProcess.functions;

	code_pointer = parentProcess.code_pointer;
}

template < typename T >
BrainThreadProcess<T>::~BrainThreadProcess(void)
{
	delete memory;
}

template < typename T >
void BrainThreadProcess<T>::Run(void)
{
	///std::cout << "run " << code_pointer << std::endl;
	while(true)
	{
		EnterCriticalSection(&critical_section);
		current_instruction = code->ToExecute(this->code_pointer);
		LeaveCriticalSection(&critical_section);

		//std::cout <<this->code_pointer <<std::endl;

		switch(current_instruction.operation)
		{
			case CodeTape::btoIncrement: 
				memory->Increment(); 
				break;
			case CodeTape::btoDecrement: 
				memory->Decrement(); 
				break;
			case CodeTape::btoMoveLeft: 
				memory->MoveLeft(); 
				break;
			case CodeTape::btoMoveRight: 
				memory->MoveRight(); 
				break;
			case CodeTape::btoStdWrite: 
				memory->Write(); 
				break;
			case CodeTape::btoStdRead: 
				memory->Read(); 
				break;
			case CodeTape::btoBeginLoop: 
				if(memory->NullCell())
				{
					code_pointer = current_instruction.jump;
				}
				break;
			case CodeTape::btoEndLoop: 
				if(memory->NullCell() == false)
				{
					code_pointer = current_instruction.jump;
				}
				break;
			case CodeTape::btoFork: 
				this->Fork();
				break;
			default:
				 
				//memory->SimpleMemoryDump();
				return;
		}

		++code_pointer;
	}

	
}

template < typename T >
void BrainThreadProcess<T>::Fork()
{
	HANDLE hThread;
	BrainThreadProcess<T> * child = new BrainThreadProcess<T>(*this);
	child->code_pointer++;
	
    hThread = (HANDLE) _beginthread( run_bt_thread<T>, 0, child );
	if(hThread == (HANDLE)-1L)
	{
		throw BFForkThreadException(errno);
	}

	//childs.Add((HANDLE)2, child);

	//childs.WaitForAll();
}

// Explicit template instantiation
template class BrainThreadProcess<char>;
template class BrainThreadProcess<unsigned char>;
template class BrainThreadProcess<unsigned short>;
template class BrainThreadProcess<unsigned int>;