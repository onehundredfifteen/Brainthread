#pragma once

#include <windows.h>
#include <vector>

/*
 * Klasa Monitora Procesów.
 * Singleton
 * Czeka na koniec wykonania wszystkich potomnych w¹tków, bo mo¿e siê zdarzyæ, ¿e g³ówny w¹tek
 * zakoñczy sie przed jego dzieæmi, a co za tym idzie - zamknie siê aplikacja.
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