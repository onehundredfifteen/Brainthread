#include <process.h>
#include "BrainThreadProcess.h"
#include "BrainThreadRuntimeException.h"


extern CRITICAL_SECTION critical_section;
extern CRITICAL_SECTION cout_critical_section;

#include <iostream>

template < typename T >
void __cdecl  run_bt_thread(void * arg) 
{
	BrainThreadProcess<T>* proc = reinterpret_cast< BrainThreadProcess<T>* >(arg) ;
	try
	{
		proc->Run();
	}
	catch(BrainThreadRuntimeException &re)
	{
		EnterCriticalSection(&cout_critical_section);
		std::cout << "ThreadId:" <<  GetCurrentThreadId() << "> "<< re.what() << std::endl;
		LeaveCriticalSection(&cout_critical_section);
	}
	catch(std::exception &e)
	{
		EnterCriticalSection(&cout_critical_section);
		std::cout << "ThreadId:" <<  GetCurrentThreadId() << "> "<< e.what() << std::endl;
		LeaveCriticalSection(&cout_critical_section);
	}
	catch(...)
	{
		EnterCriticalSection(&cout_critical_section);
		std::cout << "ThreadId:" <<  GetCurrentThreadId() << "> FATAL ERROR" << std::endl;
		LeaveCriticalSection(&cout_critical_section);
	}
	
	delete proc;
	proc = NULL;
	_endthread();
}

template < typename T >
BrainThreadProcess<T>::BrainThreadProcess(ProcessMonitor * monitor, CodeTape * c, /*res_context r_ctx,*/ MemoryHeap<T> *shared_heap, unsigned int mem_size, typename MemoryTape<T>::mem_option mo, typename MemoryTape<T>::eof_option eo)
{
	 memory = nullptr;

	 process_monitor = monitor;
	 code = c;
	// resource_context = rctx;
	 memory = new MemoryTape<T>(mem_size, eo, mo);
	 this->shared_heap = shared_heap;
	 
	 code_pointer = 0;
}

template < typename T >
BrainThreadProcess<T>::BrainThreadProcess(const BrainThreadProcess<T> &parentProcess)
{
	memory = nullptr;
	
	process_monitor = parentProcess.process_monitor;
//	resource_context = parentProcess.resource_context;
	code = parentProcess.code;
	memory = new MemoryTape<T>(*parentProcess.memory);
	shared_heap = parentProcess.shared_heap;
	functions = parentProcess.functions;

	code_pointer = parentProcess.code_pointer;
}

template < typename T >
BrainThreadProcess<T>::~BrainThreadProcess(void)
{
	if(memory)	
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
			case CodeTape::btoWait: 
				this->Join();
				break;
			default:
				 
				//memory->SimpleMemoryDump();
				return;
		}

		++code_pointer;

		Sleep( 0 ); // reszta czasu dla innych w¹tków
	}

	
}

template < typename T >
void BrainThreadProcess<T>::Fork()
{
	HANDLE hThread;
	BrainThreadProcess<T> * child = new BrainThreadProcess<T>(*this);

	this->memory->AfterFork(child->memory);
	++child->code_pointer;
	
    hThread = (HANDLE) _beginthread( run_bt_thread<T>, 0, child );

	if(hThread == (HANDLE)-1L)
	{
		delete child;
		throw BFForkThreadException(errno);
	}

	child_threads.push_back(hThread);
	process_monitor->AddProcess(hThread);
}

template < typename T >
void BrainThreadProcess<T>::Join(void)
{ 
	if(child_threads.size())
	{
		if(child_threads.size() > MAXIMUM_WAIT_OBJECTS)
			throw BFJoinThreadException(0);
	
		DWORD result =  WaitForMultipleObjects(child_threads.size(),
											   &child_threads[0],
											   true, //wait all
											   INFINITE
											  );

		if(result == WAIT_FAILED)
			throw BFJoinThreadException(::GetLastError());
	}
}

// Explicit template instantiation
template class BrainThreadProcess<char>;
template class BrainThreadProcess<unsigned char>;
template class BrainThreadProcess<unsigned short>;
template class BrainThreadProcess<unsigned int>;