// Brainthread.cpp : Defines the entry point for the console application.
//
#include <windows.h>
#include <process.h>

#include "BrainThread.h"
#include "Parser.h"
#include "Debuger.h"
#include "LogStream.h"

CRITICAL_SECTION code_critical_section;
CRITICAL_SECTION cout_critical_section;
CRITICAL_SECTION pm_critical_section;
CRITICAL_SECTION heap_critical_section;

LogStream *log_stream;



int main(int argc, char* argv[])
{
	InitializeCriticalSection(&code_critical_section);
	InitializeCriticalSection(&cout_critical_section);
	InitializeCriticalSection(&pm_critical_section);
	InitializeCriticalSection(&heap_critical_section);

	
	MessageLog messages(true);
	log_stream = new LogStream(LogStream::lsFile);
	log_stream->SetLogPath("log.txt");

	Parser pa(&messages), pa2(&messages, true);

	 messages.AddMessage(MessageLog::ecInformation,"Parsing start");

	try
	{
		
		pa2.Parse("++M>");
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

	messages.AddMessage(MessageLog::ecInformation,"Parse end");

	#define TYPEMOI unsigned short
	
    messages.AddMessage(MessageLog::ecInformation,"Debug start");

	Debuger d(&messages, pa2.GetCode(), sizeof(TYPEMOI), true);
	d.Debug();

	messages.AddMessage(MessageLog::ecInformation,"Debug end");
	
	
	
	
	CodeTape c, c2;


	//pa.GetCode(c);
	pa2.GetCode(c);
	BrainThread<TYPEMOI> p;
	p.mem_size = 5000;

	p.mem_behavior = MemoryTape<TYPEMOI>::moDynamic;
	p.eof_behavior = MemoryTape<TYPEMOI>::eoZero;
	p.resource_context  = BrainThreadProcess<TYPEMOI>::rcShared;
	
	p.Run(&c);
	p.WaitForPendingThreads();
	
	messages.AddMessage(MessageLog::ecMessage,"Execution end");
	messages.GetMessages();
	//todo compiler info _line_ date
	system("pause");
	DeleteCriticalSection(&code_critical_section);
	DeleteCriticalSection(&cout_critical_section);
	DeleteCriticalSection(&pm_critical_section);
	DeleteCriticalSection(&heap_critical_section);
	delete log_stream;

	return 0;
}
