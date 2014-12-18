#pragma once

#include <windows.h>
#include <list>

/*
 * Klasa Monitora Proces�w.
 * Czeka na koniec wykonania wszystkich potomnych w�tk�w, bo mo�e si� zdarzy�, �e g��wny w�tek
 * zako�czy sie przed jego dzie�mi, a co za tym idzie - zamknie si� aplikacja.
*/

class ProcessMonitor
{
public:
	ProcessMonitor(void){};
	~ProcessMonitor(void){};

	void AddProcess(HANDLE h);
	void WaitForWorkingProcesses(void);

protected:
	std::list<HANDLE> handles; 

	bool RemoveTerminatedProcesses(void);

	static const unsigned wait_time = 1000;
};