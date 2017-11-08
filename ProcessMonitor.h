#pragma once

#include <windows.h>
#include <vector>

/*
 * Klasa Monitora Proces�w.
 * Singleton
 * Czeka na koniec wykonania wszystkich potomnych w�tk�w, bo mo�e si� zdarzy�, �e g��wny w�tek
 * zako�czy sie przed jego dzie�mi, a co za tym idzie - zamknie si� aplikacja.
*/

extern CRITICAL_SECTION pm_critical_section;

class ProcessMonitor
{
public:
	static ProcessMonitor & Instance()
	{
		// it **is** thread-safe in C++11.
		static ProcessMonitor instance;

		return instance;
	}

private:
	  std::vector<HANDLE> handles; 
	  static unsigned int threads_cnt;

	  ProcessMonitor(){ 
		 handles.reserve(10); 
	  }
	  ~ProcessMonitor(void){};

	  ProcessMonitor(ProcessMonitor const&);             
	  ProcessMonitor& operator=(ProcessMonitor const&); 

public:
	  static inline void EnterCriticalSection(CRITICAL_SECTION &cs)
	  {
		 if(threads_cnt > 0)
			::EnterCriticalSection(&cs);
	  }

	  static inline void LeaveCriticalSection(CRITICAL_SECTION &cs)
	  {
		 if(threads_cnt > 0)
		 {
			::LeaveCriticalSection(&cs);
		 }
	  }
public:
		void AddProcess(HANDLE h);
		void WaitForWorkingProcesses(void);

		unsigned int GetProcessId(HANDLE h);
};