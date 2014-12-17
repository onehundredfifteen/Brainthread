#pragma once

#include <Windows.h>
#include <vector>

template < typename T >
class BrainThreadProcess;

template < typename T >
class ChildProcesses
{
	public:
		ChildProcesses(void);
		~ChildProcesses(void);

		void Add(HANDLE h, BrainThreadProcess<T> *process);
		void WaitForAll(void);
	
		//void PrintStackTrace();

	protected:
		std::vector< BrainThreadProcess<T> *  > processes;
		std::vector< HANDLE > handles;
};