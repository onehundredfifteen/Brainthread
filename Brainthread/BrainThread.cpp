#include "BrainThread.h"
#include "BrainThreadRuntimeException.h"

#include <iostream>


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
	
		main_process = new BrainThreadProcess<T>(c, &shared_heap, mem_size, mem_behavior, eof_behavior);
		main_process->Run();
		//main_process->childs.WaitForAll();
	}
	catch(BrainThreadRuntimeException &re)
	{
		std::cout << re.what() << std::endl;
	}
	catch(std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}
	catch(...)
	{
		std::cout << "Powazny blad" << std::endl;
	}
}

// Explicit template instantiation
template class BrainThread<char>;
template class BrainThread<unsigned char>;
template class BrainThread<unsigned short>;
template class BrainThread<unsigned int>;