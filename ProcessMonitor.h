#pragma once

#include <windows.h>
#include <vector>

/*
 * Klasa Monitora Procesów.
 * Singleton
 * Czeka na koniec wykonania wszystkich potomnych w¹tków, bo mo¿e siê zdarzyæ, ¿e g³ówny w¹tek
 * zakoñczy sie przed jego dzieæmi, a co za tym idzie - zamknie siê aplikacja.
*/

extern CRITICAL_SECTION pm_critical_section;

class ProcessMonitor
{
public:
	static ProcessMonitor & Instance()
	{
		static ProcessMonitor instance;
		return instance;
	}

private:
	  std::vector<HANDLE> handles; 

	  ProcessMonitor(){ 
		 handles.reserve(10); 
	  }

	  ProcessMonitor(ProcessMonitor const&) = delete;
	  ProcessMonitor& operator=(ProcessMonitor const&) = delete; 

public:
	  static inline void EnterCriticalSection(CRITICAL_SECTION &cs)
	  {
		 if(Instance().handles.size() > 0)
			::EnterCriticalSection(&cs);
	  }

	  static inline void LeaveCriticalSection(CRITICAL_SECTION &cs)
	  {
		 if(Instance().handles.size() > 0)		 {
			::LeaveCriticalSection(&cs);
		 }
	  }
public:
		void AddProcess(HANDLE h);
		void WaitForWorkingProcesses(void);

		unsigned int GetProcessId(HANDLE h);
};