#include <process.h>

#include "ProcessMonitor.h"
#include "BrainThreadProcess.h"
#include "BrainThreadRuntimeException.h"
#include "DebugLogStream.h"

extern CRITICAL_SECTION cout_critical_section;
extern CRITICAL_SECTION heap_critical_section;

using namespace CodeTape;

template < typename T >
unsigned int __stdcall run_bt_thread(void * arg) 
{
	BrainThreadProcess<T>* process = reinterpret_cast< BrainThreadProcess<T>* >(arg) ;
	try
	{
		process->Run();
	}
	catch(BrainThreadRuntimeException &re)
	{
		::EnterCriticalSection(&cout_critical_section);
		std::cerr << "<t" <<  GetCurrentThreadId() << "> "<< re.what() << std::endl;
		::LeaveCriticalSection(&cout_critical_section);
	}
	catch(std::exception &e)
	{
		::EnterCriticalSection(&cout_critical_section);
		std::cerr << "<t" <<  GetCurrentThreadId() << "> "<< e.what() << std::endl;
		::LeaveCriticalSection(&cout_critical_section);
	}
	catch(...)
	{
		::EnterCriticalSection(&cout_critical_section);
		std::cerr << "<t" <<  GetCurrentThreadId() << "> FATAL ERROR" << std::endl;
		::LeaveCriticalSection(&cout_critical_section);
	}

	_endthreadex(0);

	delete process;
	process = nullptr;

	return 0;
}

template < typename T >
BrainThreadProcess<T>::BrainThreadProcess(const CodeTape::Tape &ctape, MemoryHeap<T> &_shared_heap, unsigned int mem_size, typename MemoryTape<T>::mem_option mo, typename MemoryTape<T>::eof_option eo)
	: isMain(true), code(ctape), shared_heap(_shared_heap), memory(mem_size, eo, mo)
{
	 code_pointer = 0;
	 child_threads.reserve(4);
}

template < typename T >
BrainThreadProcess<T>::BrainThreadProcess(const BrainThreadProcess<T> &parentProcess) 
	: isMain(false), code(parentProcess.code), shared_heap(parentProcess.shared_heap), memory(parentProcess.memory)
{
	code_pointer = parentProcess.code_pointer;
	child_threads.reserve(4);
}

template < typename T >
BrainThreadProcess<T>::~BrainThreadProcess(void)
{

}

template < typename T >
void BrainThreadProcess<T>::Run(void)
{
	bt_instruction current_instruction;
	int opt = true;

	while(true)
	{
		//ProcessMonitor::EnterCriticalSection(code_critical_section);
		current_instruction = code[this->code_pointer];
		//ProcessMonitor::LeaveCriticalSection(code_critical_section);

		switch(current_instruction.operation)
		{
			case bt_operation::btoIncrement:
				opt ? memory.Increment(current_instruction.repetitions) : memory.Increment();
				break;
			case bt_operation::btoDecrement:
				opt ? memory.Decrement(current_instruction.repetitions) : memory.Decrement();
				break;
			case bt_operation::btoMoveLeft:
				opt ? memory.MoveLeft(current_instruction.repetitions) : memory.MoveLeft();
				break;
			case bt_operation::btoMoveRight:
				opt ? memory.MoveRight(current_instruction.repetitions) : memory.MoveRight();
				break;
			case bt_operation::btoAsciiWrite:
				memory.Write(); 
				break;
			case bt_operation::btoAsciiRead:
				memory.Read(); 
				break;
			case bt_operation::btoDecimalWrite:
				memory.DecimalWrite(); 
				break;
			case bt_operation::btoDecimalRead:
				memory.DecimalRead(); 
				break;
			case bt_operation::btoBeginLoop:
				if(*(this->memory.GetValue()) == 0)
				{
					code_pointer = current_instruction.jump;
				}
				break;
			case bt_operation::btoEndLoop:
				if(*(this->memory.GetValue()) != 0)
				{
					code_pointer = current_instruction.jump;
				}
				break;
            case bt_operation::btoBeginFunction:

				this->functions.Add(*(this->memory.GetValue()), code_pointer);
				code_pointer = current_instruction.jump;
				break;
			case bt_operation::btoEndFunction:

				if(this->functions.Return(&code_pointer) == false && isMain == false)//terminate threads spawned within function
					return;

				break;
			case bt_operation::btoCallFunction:

				this->functions.Call(*(this->memory.GetValue()), &code_pointer);
				--code_pointer; //bo na ko�cu p�tli jest ++
				break;
			case bt_operation::btoFork:
				this->Fork();
				break;
			case bt_operation::btoJoin: 
				this->Join();
				break;
            case bt_operation::btoTerminate:
				return; // :)
			case bt_operation::btoPush: 
				this->heap.Push( *(this->memory.GetValue()) );
				break;
			case bt_operation::btoPop: 
				*(this->memory.GetValue()) = this->heap.Pop();
				break;
            case bt_operation::btoSwap: 
				this->heap.Swap();
				break;
			case bt_operation::btoSharedPush: 

				ProcessMonitor::EnterCriticalSection(heap_critical_section);
				this->shared_heap.Push( *(this->memory.GetValue()) );
				ProcessMonitor::LeaveCriticalSection(heap_critical_section);
				break;
			case bt_operation::btoSharedPop: 

				ProcessMonitor::EnterCriticalSection(heap_critical_section);
				*(this->memory.GetValue()) = this->shared_heap.Pop();
				ProcessMonitor::LeaveCriticalSection(heap_critical_section);
				break;
            case bt_operation::btoSharedSwap: 

				ProcessMonitor::EnterCriticalSection(heap_critical_section);
				this->shared_heap.Swap();
				ProcessMonitor::LeaveCriticalSection(heap_critical_section);
				break;

			/***********************
			debug instructions
			************************/
			case bt_operation::btoDEBUG_SimpleMemoryDump: 
					ProcessMonitor::EnterCriticalSection(cout_critical_section);
					this->memory.SimpleMemoryDump(DebugLogStream::Instance().GetStream());
					ProcessMonitor::LeaveCriticalSection(cout_critical_section);
				break;
			case bt_operation::btoDEBUG_MemoryDump: 
					ProcessMonitor::EnterCriticalSection(cout_critical_section);
					this->memory.MemoryDump(DebugLogStream::Instance().GetStream());
					ProcessMonitor::LeaveCriticalSection(cout_critical_section);
				break;
			case bt_operation::btoDEBUG_StackDump: 
					ProcessMonitor::EnterCriticalSection(cout_critical_section);
					this->heap.PrintStack(DebugLogStream::Instance().GetStream());
					ProcessMonitor::LeaveCriticalSection(cout_critical_section);
				break;
			case bt_operation::btoDEBUG_SharedStackDump: 
					ProcessMonitor::EnterCriticalSection(cout_critical_section);
					this->shared_heap.PrintStack(DebugLogStream::Instance().GetStream());
					ProcessMonitor::LeaveCriticalSection(cout_critical_section);
				break;
			case bt_operation::btoDEBUG_FunctionsStackDump: 
					ProcessMonitor::EnterCriticalSection(cout_critical_section);
					this->functions.PrintStackTrace(DebugLogStream::Instance().GetStream());
					ProcessMonitor::LeaveCriticalSection(cout_critical_section);
				break;
			case bt_operation::btoDEBUG_FunctionsDefsDump: 
					ProcessMonitor::EnterCriticalSection(cout_critical_section);
					this->functions.PrintDeclaredFunctions(DebugLogStream::Instance().GetStream());
					ProcessMonitor::LeaveCriticalSection(cout_critical_section);
				break;
			case bt_operation::btoDEBUG_ThreadInfoDump: 
					ProcessMonitor::EnterCriticalSection(cout_critical_section);
					this->PrintProcessInfo(DebugLogStream::Instance().GetStream());
					ProcessMonitor::LeaveCriticalSection(cout_critical_section);
				break;
			
				// Optimizer
			case bt_operation::btoOPT_SetCellToZero:
				*(this->memory.GetValue()) = 0;
				break;

			case bt_operation::btoOPT_NoOperation:
			case bt_operation::btoSwitchHeap: 
				break;
			/***********************
			end debug instructions
			************************/
			default:	
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
	BrainThreadProcess<T> * child = nullptr;

	try
	{
		*(this->memory.GetValue()) = 0;

		child = new BrainThreadProcess<T>(*this);
		//*(this->memory->GetValue()) = 0;
		//*(child->memory->GetValue()) = 0;
		
		child->memory.MoveRight();
		*(child->memory.GetValue()) = 1;
		++child->code_pointer;
	}
	catch(const BFRangeException &re)
	{
		delete child;
		throw re;
	}
	catch(const std::bad_alloc &ba)
	{
		//delete child;
		throw BFForkThreadException(ERROR_CODE_NOTENOUGHMEMORY);
	}
	catch(...)
	{
		delete child;
		throw BFForkThreadException(::GetLastError());
	}
	
    hThread = (HANDLE) _beginthreadex(NULL, 64000, run_bt_thread<T>, child, 0, NULL);//mozna dac najmniej 64kb 64*1024

	if(hThread <= (HANDLE)0L)
	{
		delete child;
		throw BFForkThreadException(::GetLastError());
	}

	try
	{
		child_threads.push_back(hThread);

	    ProcessMonitor::Instance().AddProcess(hThread);
	}
	catch(...)
	{
		throw BFCannotMonitorThreadsException();
	}
}

template < typename T >
void BrainThreadProcess<T>::Join(void)
{ 
	DWORD result;

	if(child_threads.size())
	{
		if(child_threads.size() > MAXIMUM_WAIT_OBJECTS)
			throw BFJoinThreadException(ERROR_CODE_TOOMANYTHREADSTOWAIT);
	
		result =  WaitForMultipleObjects(child_threads.size(),
											   &child_threads[0],
											   true, //wait all
											   INFINITE
											  );

		if(result == WAIT_FAILED)
			throw BFJoinThreadException(::GetLastError());

		child_threads.erase(child_threads.begin(), child_threads.end());
	}
}

template < typename T >
std::ostream& BrainThreadProcess<T>::PrintProcessInfo(std::ostream &s)
{
	std::vector<HANDLE>::iterator it;
	int i = 0;
	int pid = ProcessMonitor::Instance().GetProcessId( GetCurrentThread() ); 

	
	s << "\n>Thread id/sys_id: " << pid << "/" << GetCurrentThreadId();

	if(pid == 1)
		s << "(main)";
	
	s << ".\nActive child threads: " << child_threads.size() << ", in order of apperance:\n";

	for(it = child_threads.begin(); it < child_threads.end(); ++it)
	{
		s << (++i) 
		  << ". id: " << ProcessMonitor::Instance().GetProcessId(*it) 
		  << " sys_id: " << ::GetThreadId(*it) 
		  << " state: " << (WaitForSingleObject(*it, 0) == WAIT_FAILED ? "error" : "running") << "\n";
		//WaitForSingleObject z zerem w 2gim parametrze s�u�y odczytaniu stanu - nie czeka
	}

	return s << std::flush;
}

template < typename T >
unsigned int BrainThreadProcess<T>::GetProcessId(void)
{
	return ProcessMonitor::Instance().GetProcessId( GetCurrentThread() );
}


// Explicit template instantiation
template class BrainThreadProcess<char>;
template class BrainThreadProcess<unsigned char>;
template class BrainThreadProcess<unsigned short>;
template class BrainThreadProcess<unsigned int>;
template class BrainThreadProcess<short>;
template class BrainThreadProcess<int>;