#include <process.h>

#include "ProcessMonitor.h"
#include "BrainThreadProcess.h"
#include "BrainThreadRuntimeException.h"
#include "DebugLogStream.h"

extern CRITICAL_SECTION heap_critical_section;

namespace BT {

	template < typename T >
	unsigned int __stdcall run_bt_thread(void* arg)
	{
		BrainThreadProcess<T>* process = reinterpret_cast<BrainThreadProcess<T>*>(arg);
		try
		{
			process->Run();
		}
		catch (const BrainThreadRuntimeException& re)
		{
			std::cerr << "<t" << GetCurrentThreadId() << "> " << re.what() << std::endl;
		}
		catch (const std::exception& e)
		{
			std::cerr << "<t" << GetCurrentThreadId() << "> " << e.what() << std::endl;
		}
		catch (...)
		{
			std::cerr << "<t" << GetCurrentThreadId() << "> FATAL ERROR" << std::endl;
		}

		_endthreadex(0);

		delete process;
		process = nullptr;

		return 0;
	}

	template < typename T >
	BrainThreadProcess<T>::BrainThreadProcess(const CodeTape& ctape, MemoryHeap<T>& _shared_heap, unsigned int mem_size, mem_option mo, eof_option eo)
		: isMain(true), code(ctape), shared_heap(_shared_heap), memory(mem_size, eo, mo)
	{
		code_pointer = 0;
		child_threads.reserve(4);
	}

	template < typename T >
	BrainThreadProcess<T>::BrainThreadProcess(const BrainThreadProcess<T>& parentProcess)
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
		while (true)
		{
			const bt_instruction & current_instruction = code[this->code_pointer];
			
			switch (current_instruction.operation)
			{
			case bt_operation::btoIncrement:
				memory.Increment();
				break;
			case bt_operation::btoDecrement:
				memory.Decrement();
				break;
			case bt_operation::btoMoveLeft:
				memory.MoveLeft();
				break;
			case bt_operation::btoMoveRight:
				memory.MoveRight();
				break;
			case bt_operation::btoOPT_Increment:
				memory.Increment(current_instruction.repetitions);
				break;
			case bt_operation::btoOPT_Decrement:
				memory.Decrement(current_instruction.repetitions);
				break;
			case bt_operation::btoOPT_MoveLeft:
				memory.MoveLeft(current_instruction.repetitions);
				break;
			case bt_operation::btoOPT_MoveRight:
				memory.MoveRight(current_instruction.repetitions);
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
				if (*(this->memory.GetValue()) == 0){
					code_pointer = current_instruction.jump;
				}
				break;
			case bt_operation::btoEndLoop:
				if (*(this->memory.GetValue()) != 0){
					code_pointer = current_instruction.jump;
				}
				break;
			case bt_operation::btoBeginFunction:
				this->functions.Add(*(this->memory.GetValue()), code_pointer);
				code_pointer = current_instruction.jump;
				break;
			case bt_operation::btoEndFunction:
				if (this->functions.Return(&code_pointer) == false && isMain == false)//terminate threads spawned within function
					return;
				break;
			case bt_operation::btoCallFunction:
				this->functions.Call(*(this->memory.GetValue()), &code_pointer);
				--code_pointer; //bo na koñcu pêtli jest ++
				break;
			case bt_operation::btoFork:
				this->Fork();
				break;
			case bt_operation::btoJoin:
				this->Join();
				break;
			case bt_operation::btoTerminate:
				return; 
			case bt_operation::btoPush:
				this->heap.Push(*(this->memory.GetValue()));
				break;
			case bt_operation::btoPop:
				*(this->memory.GetValue()) = this->heap.Pop();
				break;
			case bt_operation::btoSwap:
				this->heap.Swap();
				break;
			case bt_operation::btoSharedPush:

				ProcessMonitor::EnterCriticalSection(heap_critical_section);
				this->shared_heap.Push(*(this->memory.GetValue()));
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
				this->memory.SimpleMemoryDump(DebugLogStream::Instance().GetStream());
				break;
			case bt_operation::btoDEBUG_MemoryDump:
				this->memory.MemoryDump(DebugLogStream::Instance().GetStream());
				break;
			case bt_operation::btoDEBUG_StackDump:
				this->heap.PrintStack(DebugLogStream::Instance().GetStream());
				break;
			case bt_operation::btoDEBUG_SharedStackDump:
				this->shared_heap.PrintStack(DebugLogStream::Instance().GetStream());
				break;
			case bt_operation::btoDEBUG_FunctionsStackDump:
				this->functions.PrintStackTrace(DebugLogStream::Instance().GetStream());
				break;
			case bt_operation::btoDEBUG_FunctionsDefsDump:
				this->functions.PrintDeclaredFunctions(DebugLogStream::Instance().GetStream());
				break;
			case bt_operation::btoDEBUG_ThreadInfoDump:
				this->PrintProcessInfo(DebugLogStream::Instance().GetStream());
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
			Sleep(0); // reszta czasu dla innych w¹tków
		}
	}

	template < typename T >
	void BrainThreadProcess<T>::Fork()
	{
		HANDLE hThread;
		BrainThreadProcess<T>* child = nullptr;

		try
		{
			*(this->memory.GetValue()) = 0;

			child = new BrainThreadProcess<T>(*this);

			child->memory.MoveRight();
			*(child->memory.GetValue()) = 1;
			++child->code_pointer;
		}
		catch (const BFRangeException& re)
		{
			delete child;
			throw re;
		}
		catch (const std::bad_alloc&)
		{
			//delete child;
			throw BFForkThreadException(ERROR_CODE_NOTENOUGHMEMORY);
		}
		catch (...)
		{
			delete child;
			throw BFForkThreadException(::GetLastError());
		}

		hThread = (HANDLE)_beginthreadex(NULL, 64000, run_bt_thread<T>, child, 0, NULL);//mozna dac najmniej 64kb 64*1024

		if (hThread <= (HANDLE)0L)
		{
			delete child;
			throw BFForkThreadException(::GetLastError());
		}

		try
		{
			child_threads.push_back(hThread);

			ProcessMonitor::Instance().AddProcess(hThread);
		}
		catch (...)
		{
			throw BFCannotMonitorThreadsException();
		}
	}

	template < typename T >
	void BrainThreadProcess<T>::Join(void)
	{
		DWORD result;

		if (child_threads.size())
		{
			if (child_threads.size() > MAXIMUM_WAIT_OBJECTS)
				throw BFJoinThreadException(ERROR_CODE_TOOMANYTHREADSTOWAIT);

			result = WaitForMultipleObjects(child_threads.size(),
				&child_threads[0],
				true, //wait all
				INFINITE
			);

			if (result == WAIT_FAILED)
				throw BFJoinThreadException(::GetLastError());

			child_threads.erase(child_threads.begin(), child_threads.end());
		}
	}

	template < typename T >
	std::ostream& BrainThreadProcess<T>::PrintProcessInfo(std::ostream& s)
	{
		std::vector<HANDLE>::iterator it;
		int i = 0;
		int pid = ProcessMonitor::Instance().GetProcessId(GetCurrentThread());


		s << "\n>Thread id/sys_id: " << pid << "/" << GetCurrentThreadId();

		if (pid == 1)
			s << "(main)";

		s << ".\nActive child threads: " << child_threads.size() << ", in order of apperance:\n";

		for (it = child_threads.begin(); it < child_threads.end(); ++it)
		{
			s << (++i)
				<< ". id: " << ProcessMonitor::Instance().GetProcessId(*it)
				<< " sys_id: " << ::GetThreadId(*it)
				<< " state: " << (WaitForSingleObject(*it, 0) == WAIT_FAILED ? "error" : "running") << "\n";
			//WaitForSingleObject z zerem w 2gim parametrze s³u¿y odczytaniu stanu - nie czeka
		}

		return s << std::flush;
	}

	template < typename T >
	unsigned int BrainThreadProcess<T>::GetProcessId(void)
	{
		return ProcessMonitor::Instance().GetProcessId(GetCurrentThread());
	}


	// Explicit template instantiation
	template class BrainThreadProcess<char>;
	template class BrainThreadProcess<unsigned char>;
	template class BrainThreadProcess<unsigned short>;
	template class BrainThreadProcess<unsigned int>;
	template class BrainThreadProcess<short>;
	template class BrainThreadProcess<int>;
}