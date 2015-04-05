#include "ProcessMonitor.h"

#include <algorithm>

extern CRITICAL_SECTION pm_critical_section;

/*
 * Klasa Monitora Proces�w.
 * Czeka na koniec wykonania wszystkich potomnych w�tk�w, bo mo�e si� zdarzy�, �e g��wny w�tek
 * zako�czy sie przed jego dzie�mi, a co za tym idzie - zamknie si� aplikacja.
*/

//Funkcja dodaje uchwyt do w�tku do listy w spos�b bezpieczny dla w�tk�w
void ProcessMonitor::AddProcess(HANDLE h)
{
	EnterCriticalSection(&pm_critical_section);
	handles.push_back(h);
	LeaveCriticalSection(&pm_critical_section);
}

//Funkcja usuwa wykonane w�tki i niszczy uchwyty.
//Zwraca true gdy istniej� pracuj�ce watki.
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
			//usuwnaie '�yj�cych' uchwyt�w
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

//Funkcja ma funkcjonalno�c WaitForMultipleObjects - czeka N ms, dop�ki wszystkie w�tki si� nie zako�cz�.
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
	
