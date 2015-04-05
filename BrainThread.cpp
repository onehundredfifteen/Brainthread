#include "BrainThread.h"
#include "BrainThreadRuntimeException.h"


#include <iostream>

extern CRITICAL_SECTION add_thr_critical_section;
extern CRITICAL_SECTION cout_critical_section;

template < typename T >
BrainThread<T>::BrainThread()
{
	 main_process = nullptr;
}

template < typename T >
BrainThread<T>::~BrainThread(void)
{
	if(main_process)
		delete main_process;
}

template < typename T >
void BrainThread<T>::Run(CodeTape * c)
{
	try
	{
		main_process = new BrainThreadProcess<T>(&process_monitor, c, &shared_heap, mem_size, mem_behavior, eof_behavior);
		main_process->Run();
	}
	catch(const BrainThreadRuntimeException &re)
	{
		EnterCriticalSection(&cout_critical_section);
		std::cout << "<main> "<< re.what() << std::endl;
		LeaveCriticalSection(&cout_critical_section);
	}
	catch(std::exception &e)
	{
		EnterCriticalSection(&cout_critical_section);
		std::cout << "<main> "<< e.what() << std::endl;
		LeaveCriticalSection(&cout_critical_section);
	}
	catch(...)
	{
		EnterCriticalSection(&cout_critical_section);
		std::cout << "<main> FATAL ERROR" << std::endl;
		LeaveCriticalSection(&cout_critical_section);
	}
	delete main_process;
	main_process = nullptr;
}

template < typename T >
void BrainThread<T>::WaitForPendingThreads(void)
{
	process_monitor.WaitForWorkingProcesses();
	//todo GetThreadTimes 
}

// Explicit template instantiation
template class BrainThread<char>;
template class BrainThread<unsigned char>;
template class BrainThread<unsigned short>;
template class BrainThread<unsigned int>;
template class BrainThread<short>;
template class BrainThread<int>;