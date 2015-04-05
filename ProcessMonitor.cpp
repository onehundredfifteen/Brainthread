#include "ProcessMonitor.h"

#include <algorithm>

extern CRITICAL_SECTION pm_critical_section;

/*
 * Klasa Monitora Procesów.
 * Czeka na koniec wykonania wszystkich potomnych w¹tków, bo mo¿e siê zdarzyæ, ¿e g³ówny w¹tek
 * zakoñczy sie przed jego dzieæmi, a co za tym idzie - zamknie siê aplikacja.
*/

//Funkcja dodaje uchwyt do w¹tku do listy w sposób bezpieczny dla w¹tków
void ProcessMonitor::AddProcess(HANDLE h)
{
	EnterCriticalSection(&pm_critical_section);
	handles.push_back(h);
	LeaveCriticalSection(&pm_critical_section);
}

//Funkcja usuwa wykonane w¹tki i niszczy uchwyty.
//Zwraca true gdy istniej¹ pracuj¹ce watki.
bool ProcessMonitor::RemoveTerminatedProcesses(void)
{
	DWORD res;
	unsigned working = 0;
	std::list<HANDLE>::iterator it = handles.begin();

	while (it != handles.end())
	{
		//tylko dowiedz sie o jego stan
		res = WaitForSingleObject(*it, 0);
		if(res == WAIT_OBJECT_0 || res == WAIT_FAILED) 
		{
			//usuwnaie '¿yj¹cych' uchwytów
			if(res != WAIT_FAILED) 
				CloseHandle(*it);

			handles.erase(it++);
		}
		else
		{
			++it;
			++working;
		}
	}

	return (working > 0);
}

//Funkcja ma funkcjonalnoœc WaitForMultipleObjects - czeka N ms, dopóki wszystkie w¹tki siê nie zakoñcz¹.
void ProcessMonitor::WaitForWorkingProcesses(void)
{
	while(handles.size())
	{
		if(RemoveTerminatedProcesses())
			Sleep(wait_time);
	}
}

bool ProcessMonitor::IsMainThread(HANDLE h)
{
	return std::find(handles.begin(), handles.end(), h) == handles.end();
}
	
