#include "ProcessMonitor.h"

#include <process.h>
#include <algorithm>

/*
 * Klasa Monitora Procesów.
 * Czeka na koniec wykonania wszystkich potomnych w¹tków, bo mo¿e siê zdarzyæ, ¿e g³ówny w¹tek
 * zakoñczy sie przed jego dzieæmi, a co za tym idzie - zamknie siê aplikacja.
*/

extern CRITICAL_SECTION pm_critical_section;


unsigned int __stdcall waitfor_queue_thread(void * arg)
{
	DWORD result;
	HANDLE * offset = reinterpret_cast<HANDLE *>(arg);

	result = WaitForMultipleObjects(MAXIMUM_WAIT_OBJECTS, offset, true, INFINITE);
	
	_endthreadex(result);
	return result;
}

//Funkcja dodaje uchwyt do w¹tku do listy w sposób bezpieczny dla w¹tków
void ProcessMonitor::AddProcess(HANDLE h)
{
	EnterCriticalSection(&pm_critical_section);
	handles.push_back(h);
	LeaveCriticalSection(&pm_critical_section);
}

//Funkcja czeka na w¹tki, dopóki wszystkie w¹tki siê nie zakoñcz¹.
void ProcessMonitor::WaitForWorkingProcesses(void)
{
	DWORD result;
	HANDLE * waitfor_handles; 
	int waitfor_cnt = 0;
	
	if(handles.size() < 1)
		return;
	else if(handles.size() > MAXIMUM_WAIT_OBJECTS)
	{
		waitfor_cnt = handles.size() / MAXIMUM_WAIT_OBJECTS;
		waitfor_handles = new HANDLE[ waitfor_cnt ];

		for(int i = 0; i < waitfor_cnt; ++i)
		{
			waitfor_handles[ i ] = (HANDLE)_beginthreadex(NULL, 0, &waitfor_queue_thread, &handles[i * MAXIMUM_WAIT_OBJECTS], 0, NULL);
		}

		result = WaitForMultipleObjects(waitfor_cnt, waitfor_handles, true, INFINITE);

		//in this purpose its ok
		result = WaitForMultipleObjects(handles.size() % MAXIMUM_WAIT_OBJECTS, &handles[waitfor_cnt * MAXIMUM_WAIT_OBJECTS], true, INFINITE);
	}
	else
	{
		result = WaitForMultipleObjects(handles.size(), &handles[0], true, INFINITE);
	}

	if(result != WAIT_FAILED)
	{
		for(std::vector<HANDLE>::iterator it = handles.begin(); it < handles.end(); ++it)	
		{
			CloseHandle( *it );
		}
	}
}

bool ProcessMonitor::IsMainThread(HANDLE h)
{
	return std::find(handles.begin(), handles.end(), h) == handles.end();
}
	
