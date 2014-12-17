#include "BFChildProcesses.h"

template < typename T >
ChildProcesses<T>::ChildProcesses(void)
{
}

template < typename T >
ChildProcesses<T>::~ChildProcesses(void)
{
}

template < typename T >
void ChildProcesses<T>::Add(HANDLE h, BrainThreadProcess<T> *process)
{
	processes.push_back(process);
	handles.push_back(h);
}

template < typename T >
void ChildProcesses<T>::WaitForAll(void)
{ //limit to MAXIMUM_WAIT_OBJECTS
	if(handles.size())
	{
	
	DWORD result =  WaitForMultipleObjects(handles.size(),
										   &handles[0],
										   true, //wait all
										   INFINITE
										  );
	}
}

// Explicit template instantiation
template class ChildProcesses<char>;
template class ChildProcesses<unsigned char>;
template class ChildProcesses<unsigned short>;
template class ChildProcesses<unsigned int>;