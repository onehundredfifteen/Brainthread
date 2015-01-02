#include <process.h>
#include "BrainThreadProcess.h"
#include "BrainThreadRuntimeException.h"


extern CRITICAL_SECTION code_critical_section;
extern CRITICAL_SECTION cout_critical_section;
extern CRITICAL_SECTION heap_critical_section;

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
		std::cout << "<t" <<  GetCurrentThreadId() << "> "<< re.what() << std::endl;
		LeaveCriticalSection(&cout_critical_section);
	}
	catch(std::exception &e)
	{
		EnterCriticalSection(&cout_critical_section);
		std::cout << "<t" <<  GetCurrentThreadId() << "> "<< e.what() << std::endl;
		LeaveCriticalSection(&cout_critical_section);
	}
	catch(...)
	{
		EnterCriticalSection(&cout_critical_section);
		std::cout << "<t" <<  GetCurrentThreadId() << "> FATAL ERROR" << std::endl;
		LeaveCriticalSection(&cout_critical_section);
	}
	
	delete proc;
	proc = nullptr;
	_endthread();
}

template < typename T >
BrainThreadProcess<T>::BrainThreadProcess(ProcessMonitor * monitor, CodeTape * c, /*res_context r_ctx,*/ MemoryHeap<T> *shared_heap, unsigned int mem_size, typename MemoryTape<T>::mem_option mo, typename MemoryTape<T>::eof_option eo)
{
	 memory = nullptr;
	 functions = nullptr;
	 heap = nullptr;

	 process_monitor = monitor;
	 code = c;
	// resource_context = rctx;
	 memory = new MemoryTape<T>(mem_size, eo, mo);
	 functions = new FunctionHeap<T>();
	 heap = new MemoryHeap<T>;

	 this->shared_heap = shared_heap;
	 
	 code_pointer = 0;
	 child_threads.reserve(4);
}

template < typename T >
BrainThreadProcess<T>::BrainThreadProcess(const BrainThreadProcess<T> &parentProcess)
{
	memory = nullptr;
	functions = nullptr;
	heap = nullptr;
	
	process_monitor = parentProcess.process_monitor;
//	resource_context = parentProcess.resource_context;
	code = parentProcess.code;
	memory = new MemoryTape<T>(*parentProcess.memory);

	functions = new FunctionHeap<T>(*parentProcess.functions);
	heap = new MemoryHeap<T>;

	shared_heap = parentProcess.shared_heap;

	code_pointer = parentProcess.code_pointer;
	child_threads.reserve(4);
}

template < typename T >
BrainThreadProcess<T>::~BrainThreadProcess(void)
{
	if(memory)	
		delete memory;

	if(functions)	
		delete functions;

	if(heap)	
		delete heap;
}

template < typename T >
void BrainThreadProcess<T>::Run(void)
{
	///std::cout << "run " << code_pointer << std::endl;
	while(true)
	{
		EnterCriticalSection(&code_critical_section);
		current_instruction = code->ToExecute(this->code_pointer);
		LeaveCriticalSection(&code_critical_section);

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
			case CodeTape::btoAsciiWrite: 
				memory->Write(); 
				break;
			case CodeTape::btoAsciiRead: 
				memory->Read(); 
				break;
			case CodeTape::btoDecimalWrite: 
				memory->DecimalWrite(); 
				break;
			case CodeTape::btoDecimalRead: 
				memory->DecimalRead(); 
				break;
			case CodeTape::btoBeginLoop: 
				if(*(this->memory->GetPointer()) == 0)
				{
					code_pointer = current_instruction.jump;
				}
				break;
			case CodeTape::btoEndLoop: 
				if(*(this->memory->GetPointer()) != 0)
				{
					code_pointer = current_instruction.jump;
				}
				break;
            case CodeTape::btoBeginFunction: 

				this->functions->Add(*(this->memory->GetPointer()), code_pointer);
				code_pointer = current_instruction.jump;
				break;
			case CodeTape::btoEndFunction: 
				this->functions->Return(&code_pointer);
				break;
			case CodeTape::btoCallFunction: 

				this->functions->Call(*(this->memory->GetPointer()), &code_pointer);
				--code_pointer; //bo na ko�cu p�tli jest ++
				break;
			case CodeTape::btoFork: 
				this->Fork();
				break;
			case CodeTape::btoJoin: 
				this->Join();
				break;
            case CodeTape::btoTerminate:
				return; // :)
			case CodeTape::btoPush: 
				this->heap->Push( *(this->memory->GetPointer()) );
				break;
			case CodeTape::btoPop: 
				*(this->memory->GetPointer()) = this->heap->Pop();
				break;
            case CodeTape::btoSwap: 
				this->heap->Swap();
				break;
			case CodeTape::btoSharedPush: 

				EnterCriticalSection(&heap_critical_section);
				this->shared_heap->Push( *(this->memory->GetPointer()) );
				LeaveCriticalSection(&heap_critical_section);
				break;
			case CodeTape::btoSharedPop: 

				EnterCriticalSection(&heap_critical_section);
				*(this->memory->GetPointer()) = this->shared_heap->Pop();
				LeaveCriticalSection(&heap_critical_section);
				break;
            case CodeTape::btoSharedSwap: 

				EnterCriticalSection(&heap_critical_section);
				this->shared_heap->Swap();
				LeaveCriticalSection(&heap_critical_section);
				break;

			//debug
			case CodeTape::btoDEBUG_SimpleMemoryDump: 
				this->memory->SimpleMemoryDump();
				break;
			default:
				 
				//memory->SimpleMemoryDump();
				return;
		}

		++code_pointer;

		Sleep( 0 ); // reszta czasu dla innych w�tk�w
	}

	
}

template < typename T >
void BrainThreadProcess<T>::Fork()
{
	HANDLE hThread;
	BrainThreadProcess<T> * child = new BrainThreadProcess<T>(*this);

	*(this->memory->GetPointer()) = 0;
	*(child->memory->GetPointer()) = 0;

	child->memory->MoveRight();
	*(child->memory->GetPointer()) = 1;
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

		child_threads.erase(child_threads.begin(),child_threads.end());
	}
}

// Explicit template instantiation
template class BrainThreadProcess<char>;
template class BrainThreadProcess<unsigned char>;
template class BrainThreadProcess<unsigned short>;
template class BrainThreadProcess<unsigned int>;