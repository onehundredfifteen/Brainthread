#include "BrainThread.h"
#include "BrainThreadRuntimeException.h"


#include <iostream>

extern CRITICAL_SECTION add_thr_critical_section;


template < typename T >
BrainThread<T>::BrainThread()
{
	 main_process = NULL;
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
	catch(BrainThreadRuntimeException &re)
	{
		std::cout << "ThreadId: main> "<< re.what() << std::endl;
	}
	catch(std::exception &e)
	{
		std::cout << "ThreadId: main> "<< e.what() << std::endl;
	}
	catch(...)
	{
		std::cout << "ThreadId: main> FATAL ERROR" << std::endl;
	}
}

template < typename T >
void BrainThread<T>::WaitForPendingThreads(void)
{
	process_monitor.WaitForWorkingProcesses();
}

// Explicit template instantiation
template class BrainThread<char>;
template class BrainThread<unsigned char>;
template class BrainThread<unsigned short>;
template class BrainThread<unsigned int>;