#pragma once

#include <windows.h>
#include <list>

/*
 * Klasa Monitora Procesów.
 * Czeka na koniec wykonania wszystkich potomnych w¹tków, bo mo¿e siê zdarzyæ, ¿e g³ówny w¹tek
 * zakoñczy sie przed jego dzieæmi, a co za tym idzie - zamknie siê aplikacja.
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