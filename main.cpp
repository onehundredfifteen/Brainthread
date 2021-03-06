// Brainthread.cpp : Defines the entry point for the console application.
//
#include <memory>
#include <windows.h>
#include <process.h>
#include "getoptpp/getopt_pp_standalone.h"

#include "BrainThread.h"
#include "Parser.h"
#include "CodeAnalyser.h"
#include "DebugLogStream.h"
#include "BrainThreadExceptions.h"
#include "BrainHelp.h"

//sekcje krytyczne
CRITICAL_SECTION cout_critical_section;
CRITICAL_SECTION pm_critical_section;
CRITICAL_SECTION heap_critical_section;

//flagi
bool OP_debug, OP_repair, OP_execute, OP_nopause;
MessageLog::MessageLevel OP_message;
DebugLogStream::stream_type OP_log;
std::string OP_source = "";
Parser::CodeLang OP_language;

MemoryTape<char>::mem_option OP_mem_behavior;
MemoryTape<char>::eof_option OP_eof_behavior; //reakcja na EOF z wej�cia
unsigned int OP_mem_size;
typedef enum __cellsize
{
	cs8,cs16,cs32,csu8,csu16,csu32
};
__cellsize OP_cellsize;
typedef enum __sourcetype
{
	stFile, stInput, stUrl
};
__sourcetype OP_sourcetype;

//domy�lna warto��
const unsigned int def_mem_size = 30000;

//inicjator argumentow
void InitArguments(GetOpt::GetOpt_pp &ops);

//Czy program zosta� uruchomiony z cmd.exe?
bool RanFromConsole();
//Rekacja na prwerwanie programu ctrl+break
bool CtrlHandler(DWORD fdwCtrlType);

//�atwe wrappery
bool RunParseArguments(GetOpt::GetOpt_pp &ops);
void RunParserAndDebug();
void RunProgram();
template < typename T > void RunProgram(BrainThread<T> &brain_main_thread);

//ta�ma kodu
std::unique_ptr<CodeTape> Code;

int main(int argc, char* argv[])
{
	//inicjalizacja sekcji krytycznych
	InitializeCriticalSection(&cout_critical_section);
	InitializeCriticalSection(&pm_critical_section);
	InitializeCriticalSection(&heap_critical_section);

	//inicjalizacja handlera obs�uguj�ego zamkni�cie programu skt�rem ctrl+break
	SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, true);

	//parsowanie opcji + deklaracje potrzebnych flag
	GetOpt::GetOpt_pp ops(argc, argv);
	ops.exceptions(std::ios::failbit | std::ios::eofbit);

	if (ops >> GetOpt::OptionPresent('h',"help"))
	{
		OP_nopause = RanFromConsole();
		if(argc > 2)
		{
			std::string help_opt;

			try
			{
				ops >>  GetOpt::Option('h',"help", help_opt);
				ShowHelp(help_opt);
			}
			catch(...)
			{
				ShowHelp("");
			}
		}
		else
			ShowHelp("");
	}
	else if (ops >> GetOpt::OptionPresent("info")){
		OP_nopause = RanFromConsole();
		ShowInfo();
	}
	else if(argc > 1)
	{
		if(RunParseArguments(ops) == true) 
		{
			if(OP_message != MessageLog::mlNone){
				PrintBrainThreadInfo();
			}  
			
			RunParserAndDebug();
		 
			if(Code && OP_execute == true){
					RunProgram();
			}
		}

		MessageLog::Instance().GetMessages();
	}
	else
	{
		ShowUsage();
	}
	
	if(OP_nopause == false)
		system("pause");
	
	DeleteCriticalSection(&cout_critical_section);
	DeleteCriticalSection(&pm_critical_section);
	DeleteCriticalSection(&heap_critical_section);

	return 0;
}

bool RunParseArguments(GetOpt::GetOpt_pp &ops)
{
	try
	{
		InitArguments(ops);
	}
	catch(GetOpt::TooManyArgumentsEx ex)
	{
		MessageLog::Instance().AddMessage(MessageLog::ecArgumentError, "Too many arguments");
		return false;
	}
	catch(GetOpt::GetOptEx ex)
    {
        MessageLog::Instance().AddMessage(MessageLog::ecArgumentError, "Error while parsing arguments: " + std::string(ex.what()));
		return false;
    }
	catch(BrainThreadInvalidOptionException ex)
	{
		MessageLog::Instance().AddMessage(MessageLog::ecArgumentError, std::string(ex.what()));
		return false;
	}
	catch(...)
	{
		MessageLog::Instance().AddMessage(MessageLog::ecArgumentError, "Unknown error");
		return false;
	}

	return true;
}

void RunParserAndDebug()
{
	try
	{
		//MessageLog::Instance().AddInfo("Code parsing started..");

		Parser parser(OP_language, OP_debug);

		if(OP_sourcetype == stInput)
		{
			parser.Parse(OP_source.c_str());
		}
		else if(OP_sourcetype == stFile)
		{
			std::ifstream in(OP_source);
			parser.Parse(in);
			in.close();
		}

		if(parser.isCodeValid()) //kod wygl�da w porz�dku
		{
			MessageLog::Instance().AddInfo("Code is valid");
		
			if(OP_debug)
			{
				//MessageLog::Instance().AddInfo("Code analysis started..");
				CodeAnalyser analyser(parser.GetCode(), OP_repair);
				analyser.Analyse();

				if(analyser.isCodeValid())
				{
					if(analyser.RepairedSomething() == true)
						MessageLog::Instance().AddInfo("Some bugs have been successfully fixed");

					MessageLog::Instance().AddInfo("Code is sane");
				}
				else
				{
					MessageLog::Instance().AddMessage("Code has warnings");
				}
			}	

			Code = std::unique_ptr<CodeTape>(new CodeTape(parser.GetCode()) );
		}
		else
		{
			MessageLog::Instance().AddMessage("Code has errors");
		}
		
	}
	catch(std::ios_base::failure &e)
	{
		MessageLog::Instance().AddMessage(MessageLog::ecFatalError, std::string(e.what()));
	}
	catch(...)
	{
		MessageLog::Instance().AddMessage(MessageLog::ecUnknownError, "In function RunParserAndDebug()");
	}
}

void RunProgram()
{
	LARGE_INTEGER frequency, t1, t2; // ticks
	double elapsedTime;
	char num_buffer[16];

	QueryPerformanceFrequency(&frequency); // get ticks per second
	QueryPerformanceCounter(&t1); // start timer
	
	switch(OP_cellsize)
	{
		case cs8:
		{
			BrainThread<char> brain;
			RunProgram(brain);
		}
		break;
		case cs16:
		{
			BrainThread<short> brain;
			RunProgram(brain);
		}
		break;
		case cs32:
		{
			BrainThread<int> brain;
			RunProgram(brain);
		}
		break;
		case csu8:
		{
			BrainThread<unsigned char> brain;
			RunProgram(brain);
		}
		break;
		case csu16:
		{
			BrainThread<unsigned short> brain;
			RunProgram(brain);
		}
		break;
		case csu32:
		{
			BrainThread<unsigned int> brain;
			RunProgram(brain);
		}
		break;
		default:
		{
			BrainThread<char> brain;
			RunProgram(brain);
		}
	}

	
   QueryPerformanceCounter(&t2); // stop timer
   elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart; // compute and print the elapsed time in millisec
   _itoa((int)elapsedTime, num_buffer, 10);

   MessageLog::Instance().AddInfo("Execution completed in " + std::string(num_buffer) + " miliseconds");
}

template < typename T > 
void RunProgram(BrainThread<T> &brain_main_thread)
{
	brain_main_thread.mem_size = OP_mem_size;

	brain_main_thread.mem_behavior      = static_cast< MemoryTape<T>::mem_option >(OP_mem_behavior);
	brain_main_thread.eof_behavior      = static_cast< MemoryTape<T>::eof_option >(OP_eof_behavior);
	
	brain_main_thread.Run(Code.get());
	brain_main_thread.WaitForPendingThreads();
}


void InitArguments(GetOpt::GetOpt_pp &ops)
{
	std::string op_arg;
	std::vector<std::string> op_args;
	unsigned long long op_arg_i;
	
	// -e --eof [0|255|nochange]
	if (ops >> GetOpt::OptionPresent('e',"eof"))
	{
        ops >>  GetOpt::Option('e', "eof", op_arg);
		if(op_arg == "0")
			OP_eof_behavior = MemoryTape<char>::eoZero;
		else if(op_arg == "255")
			OP_eof_behavior = MemoryTape<char>::eoMinusOne;
		else if(op_arg == "nochange")
			OP_eof_behavior = MemoryTape<char>::eoUnchanged;
		else 
			throw BrainThreadInvalidOptionException("EOF", op_arg);
	}
	else OP_eof_behavior = MemoryTape<char>::eoZero;
	
	// -c --cellsize [8|16|32|u8|u16|u32]
	if (ops >> GetOpt::OptionPresent('c',"cellsize"))
	{
        ops >>  GetOpt::Option('c',"cellsize", op_arg);
		if(op_arg == "8")
			OP_cellsize = cs8;
		else if(op_arg == "16")
			OP_cellsize = cs16;
		else if(op_arg == "32")
			OP_cellsize = cs32;
		else if(op_arg == "u8")
			OP_cellsize = csu8;
		else if(op_arg == "u16")
			OP_cellsize = csu16;
		else if(op_arg == "u32")
			OP_cellsize = csu32;
		else 
			throw BrainThreadInvalidOptionException("CELLSIZE", op_arg);
	}
	else OP_cellsize = cs8;

	// -m --memorysize <1,2^32>
	if (ops >> GetOpt::OptionPresent('m',"memorysize"))
	{
        ops >>  GetOpt::Option('m', "memorysize", op_arg);
		op_arg_i = atoi(op_arg.c_str());

		if(op_arg_i < 1 || op_arg_i > UINT_MAX )
			throw BrainThreadInvalidOptionException("MEMORYSIZE", op_arg);
		else 
			OP_mem_size = (unsigned int)op_arg_i;
	}
	else OP_mem_size = def_mem_size;

	// -b --memorybehavior [constant|tapeloop|dynamic]
	if (ops >> GetOpt::OptionPresent('b',"memorybehavior"))
	{
        ops >>  GetOpt::Option('b', "memorybehavior", op_arg);
		if(op_arg == "constant")
			OP_mem_behavior = MemoryTape<char>::moLimited;
		else if(op_arg == "tapeloop")
			OP_mem_behavior = MemoryTape<char>::moContinuousTape;
		else if(op_arg == "dynamic")
			OP_mem_behavior = MemoryTape<char>::moDynamic;
		else 
			throw BrainThreadInvalidOptionException("MEMORYBEHAVIOR", op_arg);
	}
	else OP_mem_behavior = MemoryTape<char>::moLimited;
	
	// -l --language [bt|b|bf|pb|brainthread|brainfuck|brainfork|pbrain|auto]
	if (ops >> GetOpt::OptionPresent('l',"language"))
	{
        ops >>  GetOpt::Option('l', "language", op_arg);
		if(op_arg == "bt" || op_arg == "brainthread")
			OP_language = Parser::clBrainThread;
		else if(op_arg == "b" || op_arg == "brainfuck")
			OP_language = Parser::clBrainFuck;
		else if(op_arg == "bf" || op_arg == "brainfork")
			OP_language = Parser::clBrainFork;
		else if(op_arg == "pb" || op_arg == "pbrain")
			OP_language = Parser::clPBrain;
		else if(op_arg == "auto")
			OP_language = Parser::clAuto;
		else 
			throw BrainThreadInvalidOptionException("LANGUAGE", op_arg);
	}

	//opcje �rodowiska

	//debug, repair & execute
	OP_debug = (ops >> GetOpt::OptionPresent('a', "analyse"));
	OP_repair = (ops >> GetOpt::OptionPresent('r', "repair")); //niekoniecznie chce, aby debug naprawia�
	OP_execute = (ops >> GetOpt::OptionPresent('x', "execute"));  //niekoniecznie chce, aby po debugu uruchamia�
	
	if(OP_repair)//aby by� repair, musi byc debug
		OP_debug = true;
	if(OP_debug == false)//nie debugujesz? musi byc execute
		OP_execute = true;

	//--verbose[all|important|none]
	//--wall
	//--silent
	if (ops >> GetOpt::OptionPresent("verbose") ||  ops >> GetOpt::OptionPresent("silent"))
	{
        if(ops >> GetOpt::OptionPresent("verbose"))
		{
			ops >>  GetOpt::Option("verbose", op_arg);
			if(op_arg == "all")
				OP_message = MessageLog::mlAll;
			else if(op_arg == "important")
				OP_message = MessageLog::mlImportant;
			else if(op_arg == "none")
				OP_message = MessageLog::mlNone;
			else 
				throw BrainThreadInvalidOptionException("VERBOSE", op_arg);
		}

		if (ops >> GetOpt::OptionPresent("silent"))
		{
			OP_message = MessageLog::mlNone;
		}
	}
	else
		OP_message = MessageLog::mlImportant;

	MessageLog::Instance().SetMessageLevel(OP_message);
	
	// --log [none|console|filename]
	if (ops >> GetOpt::OptionPresent("log"))
	{
		ops >>  GetOpt::Option("log", op_arg);

		if(op_arg.find(".") != std::string::npos) //rozpoznajemy �e wpisano plik
			OP_log = DebugLogStream::lsFile;
		else if(op_arg == "none")
			OP_log = DebugLogStream::lsNone;
		else if(op_arg == "console")
			OP_log = DebugLogStream::lsConsole;
		else 
			throw BrainThreadInvalidOptionException("LOG", op_arg);

		DebugLogStream::Instance().Init(OP_log, op_arg);
	}
	else
	{
		DebugLogStream::Instance().Init(DebugLogStream::lsConsole, "");
	}

	//-s --sourcefile [filename]
	//-i --input [code]
	//--sourcecode [code]
	if (ops >> GetOpt::OptionPresent('s',"sourcefile"))
	{
		ops >>  GetOpt::Option('s',"sourcefile", op_arg);

		if(op_arg.find(".") != std::string::npos) //rozpoznajemy �e wpisano plik
		{
			OP_sourcetype = stFile;
			OP_source = op_arg;
		}
		else 
			throw BrainThreadInvalidOptionException("SOURCEFILE", op_arg);
	}
	else if (ops >> GetOpt::OptionPresent('i',"input") || ops >> GetOpt::OptionPresent("sourcecode")) //-i --input --sourcefile [code]
	{
		if(ops >> GetOpt::OptionPresent('i',"input"))
			ops >>  GetOpt::Option('i',"input", op_arg);
		else
			ops >>  GetOpt::Option("sourcecode", op_arg);
		
		OP_sourcetype = stInput;
		OP_source = op_arg;
	}

	//--strict //super zgodne ustawienie opcji
	if(ops >> GetOpt::OptionPresent("strict"))
	{
		switch(OP_language)
		{
			case Parser::clBrainThread:
			case Parser::clBrainFork:
				OP_eof_behavior = MemoryTape<char>::eoMinusOne; 
			break;

			case Parser::clPBrain:
			case Parser::clBrainFuck:
			default:
				OP_eof_behavior = MemoryTape<char>::eoUnchanged;
				
		}

		OP_mem_size = def_mem_size;
		OP_cellsize = cs8;
		OP_mem_behavior = MemoryTape<char>::moLimited;
	}

	//--nopause //czy chcesz zatrzyma� program po wykonaniu
	if(ops >> GetOpt::OptionPresent("nopause") == false)
	{
		OP_nopause = RanFromConsole(); //nie ma opcji - pozw�l samemu wykminic czy jest potrzeba
	}
	else OP_nopause = true;

	//reszta opcji, w�a�ciwie spodziewany sie jednej tylko
	if(ops.options_remain())
	{
		ops >> GetOpt::GlobalOption(op_args);

		if(op_args.size() > 1)
				throw GetOpt::TooManyArgumentsEx();
		else 
		{  
			if(GetFileAttributesA(op_args[0].c_str()) == INVALID_FILE_ATTRIBUTES) 
			{
				throw GetOpt::InvalidFormatEx();
			}
			else //rozpoznajemy �e wpisano plik
			{
				OP_sourcetype = stFile;
				OP_source = op_args[0];
			}
		}
	}
}

bool RanFromConsole()
{
	HWND consoleWnd = GetConsoleWindow();
    DWORD dwProcessId;
    GetWindowThreadProcessId(consoleWnd, &dwProcessId);

    return ( !(GetCurrentProcessId() == dwProcessId ));
}

bool CtrlHandler(DWORD fdwCtrlType) 
{ 
  if( fdwCtrlType == CTRL_BREAK_EVENT ) 
  { 
	  if(OP_execute)
		  MessageLog::Instance().AddInfo("Execution interrupted by user");

	  MessageLog::Instance().GetMessages();
	  return true; //??
  } 
  return false; //pass all normally
} 

