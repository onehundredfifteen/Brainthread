// Brainthread.cpp : Defines the entry point for the console application.
//
#include "BrainThread.h"
#include "Parser.h"

#include <windows.h>
#include <process.h>

#include <iostream>

volatile int counter = 0;
CRITICAL_SECTION critical_section;
CRITICAL_SECTION cout_critical_section;



int main(int argc, char* argv[])
{
	InitializeCriticalSection(&critical_section);
	InitializeCriticalSection(&cout_critical_section);

	Parser pa, pa2;
	//pa.Parse("+++++++++++++++++++++++++++++++++.\0");
	try
	{
	pa.Parse("{+++++++++++++++++++++++++++++++++[-]+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.\0");
	pa2.Parse("{[-<]+++[>+++++<-]>[>+>+++>+>++>+++++>++<[++<]>---]>->-.[>++>+<<--]>--.--.+.>>>++.<<.<------.+.+++++.>>-.<++++.<--.>>>.<<---.<.-->-.>+.[+++++.---<]>>[.--->]<<.<+.++.++>+++[.<][.]<++.\0");
	}
	catch(std::exception e)
	{
		std::cerr << e.what() << std::endl;
	}


	if(pa2.isCodeValid() == false)
	{
		pa2.GetMessages();
		
	}
	
	
	CodeTape c, c2;
	//pa.GetCode(c);
	pa2.GetCode(c);
	BrainThread<unsigned char> p;
	p.mem_size = 30000;

	p.mem_behavior = MemoryTape<unsigned char>::moLimited;
	p.eof_behavior = MemoryTape<unsigned char>::eoEOF;
	
	p.Run(&c);
	//p2.Run();
	
	//HANDLE hThread =( HANDLE ) _beginthread( run_bt_thread<unsigned char>, 0, &p );
	//HANDLE hThread2 =( HANDLE ) _beginthread( run_bt_thread<unsigned char>, 0, &p2 );
	//HANDLE hThread3 =( HANDLE ) _beginthread( run_bt_thread, 0, &p1 );
	
	system("pause");
	DeleteCriticalSection(&critical_section);
	DeleteCriticalSection(&cout_critical_section);
	return 0;
}



/*
void __cdecl  run_bt_thread(void * arg) 
{
	((BrainThreadProcess<T>*)arg)->Run();
	_endthread();
}
*/