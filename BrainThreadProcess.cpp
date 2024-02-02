#include <mutex>

#include "BrainThreadProcess.h"
#include "BrainThreadRuntimeException.h"
#include "DebugLogStream.h"

namespace BT {
	
	template < typename T >
	BrainThreadProcess<T>::BrainThreadProcess(const CodeTape& ctape, unsigned int mem_size, mem_option mo, eof_option eo)
		: isMain(true), code(ctape), memory(mem_size, eo, mo)
	{
		code_pointer = 0;
		shared_heap = std::make_shared<MemoryHeap<T>>();
	}

	template < typename T >
	BrainThreadProcess<T>::BrainThreadProcess(const BrainThreadProcess<T>& parentProcess)
		: isMain(false), code(parentProcess.code), memory(parentProcess.memory)
	{
		code_pointer = parentProcess.code_pointer;
		shared_heap = parentProcess.shared_heap;
	}

	template < typename T >
	void BrainThreadProcess<T>::Run()
	{
		try {
			ExecInstructions();
			Join();
		}
		catch (const BrainThreadRuntimeException& re) {
			std::cerr << "<t" << std::this_thread::get_id() << "> " << re.what() << std::endl;
		}
		catch (const std::exception& e) {
			std::cerr << "<t" << std::this_thread::get_id() << "> " << e.what() << std::endl;
		}
		catch (...)	{
			std::cerr << "<t" << std::this_thread::get_id() << "> FATAL ERROR" << std::endl;
		}
	}

	template < typename T >
	void BrainThreadProcess<T>::ExecInstructions(void)
	{
		std::mutex _mutex;
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
				{
					const std::lock_guard<std::mutex> lock(_mutex);
					shared_heap->Push(*(this->memory.GetValue()));
				}
				break;
			case bt_operation::btoSharedPop:
				{
					const std::lock_guard<std::mutex> lock(_mutex);
					*(this->memory.GetValue()) = shared_heap->Pop();
				}
				break;
			case bt_operation::btoSharedSwap:
				{
					const std::lock_guard<std::mutex> lock(_mutex);
					shared_heap->Swap();
				}
				break;

				/**debug instructions
				**/
			case bt_operation::btoDEBUG_SimpleMemoryDump:
				{
					const std::lock_guard<std::mutex> lock(_mutex);
					memory.SimpleMemoryDump(DebugLogStream::Instance().GetStream());
				}
				break;
			case bt_operation::btoDEBUG_MemoryDump:
				{
					const std::lock_guard<std::mutex> lock(_mutex);
					memory.MemoryDump(DebugLogStream::Instance().GetStream());
				}
				break;
			case bt_operation::btoDEBUG_StackDump:
				{
					const std::lock_guard<std::mutex> lock(_mutex);
					heap.PrintStack(DebugLogStream::Instance().GetStream());
				}
				break;
			case bt_operation::btoDEBUG_SharedStackDump:
				{
					const std::lock_guard<std::mutex> lock(_mutex);
					shared_heap->PrintStack(DebugLogStream::Instance().GetStream());
				}	
				break;
			case bt_operation::btoDEBUG_FunctionsStackDump:
				{
					const std::lock_guard<std::mutex> lock(_mutex);
					functions.PrintStackTrace(DebugLogStream::Instance().GetStream());
				}
				break;
			case bt_operation::btoDEBUG_FunctionsDefsDump:
				{
					const std::lock_guard<std::mutex> lock(_mutex);
					functions.PrintDeclaredFunctions(DebugLogStream::Instance().GetStream());
				}
				break;
			case bt_operation::btoDEBUG_ThreadInfoDump:
				{
					const std::lock_guard<std::mutex> lock(_mutex);
					PrintProcessInfo(DebugLogStream::Instance().GetStream());
				}
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
			std::this_thread::yield(); // reszta czasu dla innych w¹tków
		}
	}

	template < typename T >
	void BrainThreadProcess<T>::Fork()
	{
		try
		{
			BrainThreadProcess<T> child(*this);

			*(this->memory.GetValue()) = 0;
			child.memory.MoveRight();
			*(child.memory.GetValue()) = 1;
			++child.code_pointer;

			child_threads.emplace_back([](BrainThreadProcess<T>& process) {
				process.Run();
			}, std::move(child));
		}
		catch (const BFRangeException& re)
		{
			throw re;
		}
		catch (const std::bad_alloc&)
		{
			throw BFForkThreadException(ERROR_CODE_NOTENOUGHMEMORY);
		}
		catch (const std::system_error& se)
		{
			throw BFForkThreadException(se.code().value());
		}
		catch (...)
		{
			throw BFForkThreadException(-1);
		}
	}

	template < typename T >
	void BrainThreadProcess<T>::Join(void)
	{
		for (std::thread& t : child_threads) {
			t.join();
		}
		child_threads.clear();
	}

	template < typename T >
	void BrainThreadProcess<T>::PrintProcessInfo(std::ostream& s)
	{
		int i = 0;
		s << "\n>Current thread id: " << std::this_thread::get_id() << " ";

		if (isMain)
			s << "(main)";  

		if (child_threads.size() == 0) {
			s << std::flush;
			return;
		}

		s << "\nChild threads: " << child_threads.size() << ", in order of apperance:";

		for (std::list<std::thread>::const_iterator it = child_threads.begin(); it != child_threads.end(); ++it)
		{
			s << '\n' << (++i)
			  << ". id: " << it->get_id()
			  << " state: " << (it->joinable() ? "running" : "joined");
		}
		s << std::flush;
	}


	// Explicit template instantiation
	template class BrainThreadProcess<char>;
	template class BrainThreadProcess<unsigned char>;
	template class BrainThreadProcess<unsigned short>;
	template class BrainThreadProcess<unsigned int>;
	template class BrainThreadProcess<short>;
	template class BrainThreadProcess<int>;
}