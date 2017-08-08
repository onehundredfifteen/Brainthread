#pragma once

#include <windows.h>
#include <vector>

/*
 * Klasa Monitora Proces�w.
 * Singleton
 * Czeka na koniec wykonania wszystkich potomnych w�tk�w, bo mo�e si� zdarzy�, �e g��wny w�tek
 * zako�czy sie przed jego dzie�mi, a co za tym idzie - zamknie si� aplikacja.
*/

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

	  ProcessMonitor(){ 
		 handles.reserve(10); 
	  }
	  ~ProcessMonitor(void){};

	  ProcessMonitor(ProcessMonitor const&);             
	  ProcessMonitor& operator=(ProcessMonitor const&); 

	public:
		void AddProcess(HANDLE h);
		void WaitForWorkingProcesses(void);
		bool IsMainThread(HANDLE h);
};