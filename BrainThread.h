#pragma once

#include <list>
#include <windows.h>

#include "BrainThreadProcess.h"
#include "ProcessMonitor.h"

template < typename T >
class BrainThread
{
public:
	BrainThread();
	~BrainThread(void);

	void Run(CodeTape * c);

	typename MemoryTape<T>::mem_option mem_behavior; //zachowanie pamieci
    typename MemoryTape<T>::eof_option eof_behavior; //reakcja na EOF z wejœcia
	typename BrainThreadProcess<T>::res_context resource_context; //wspo³dzielenie zasobów przez w¹tki
	unsigned int mem_size;

	void WaitForPendingThreads(void);

protected:
	MemoryHeap<T> shared_heap;
	BrainThreadProcess<T> *main_process;
	ProcessMonitor process_monitor;
};