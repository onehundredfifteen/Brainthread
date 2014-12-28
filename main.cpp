// Brainthread.cpp : Defines the entry point for the console application.
//
#include "BrainThread.h"
#include "Parser.h"

#include <windows.h>
#include <process.h>

#include <iostream>



CRITICAL_SECTION code_critical_section;
CRITICAL_SECTION cout_critical_section;
CRITICAL_SECTION pm_critical_section;
CRITICAL_SECTION heap_critical_section;



int main(int argc, char* argv[])
{
	InitializeCriticalSection(&code_critical_section);
	InitializeCriticalSection(&cout_critical_section);
	InitializeCriticalSection(&pm_critical_section);
	InitializeCriticalSection(&heap_critical_section);

	Parser pa, pa2(true);
	//pa.Parse("+++++++++++++++++++++++++++++++++.\0");
	try
	{
		
		pa2.Parse(">++++++++[<+++++++++>-]<.>>+>+>++>[-]+<[>[->+<<++++>]<<]>.+++++++..+++.>>+++++++.<<<[[-]<[-]>]<+++++++++++++++.>>.+++.------.--------.>>+.>++++.M");
		//pa2.Parse(",[#,]$[.$]\0");
		//pa2.Parse("{[->-[-]<<+/>M!]}\\+++++++++++++++++++++++++.");
		//pa2.Parse(";[:[-]++++++++++.;]");
		//pa2.Parse("{[<[+]]+++++++++++++++++++++++++++++++++[-]+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++}.\0");
		//pa2.Parse("{[-<]}+++[>+++++<-]>[>+>+++>+>++>+++++>++<[++<]>---]>->-.[>++>+<<--]>--.--.+.>>>++.<<.<------.+.+++++.>>-.<++++.<--.>>>.<<---.<.-->-.>+.[+++++.---<]>>[.--->]<<.<+.++.++>+++[.<][.]<++.\0");
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
#define TYPEMOI unsigned short

	//pa.GetCode(c);
	pa2.GetCode(c);
	BrainThread<TYPEMOI> p;
	p.mem_size = 20000;

	p.mem_behavior = MemoryTape<TYPEMOI>::moDynamic;
	p.eof_behavior = MemoryTape<TYPEMOI>::eoZero;
	p.resource_context  = BrainThreadProcess<TYPEMOI>::rcShared;
	
	p.Run(&c);
	p.WaitForPendingThreads();
	
	
	system("pause");
	DeleteCriticalSection(&code_critical_section);
	DeleteCriticalSection(&cout_critical_section);
	DeleteCriticalSection(&pm_critical_section);
	DeleteCriticalSection(&heap_critical_section);
	return 0;
}
