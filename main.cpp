// Brainthread.cpp : Defines the entry point for the console application.
//
#include <windows.h>
#include <process.h>
#include "getoptpp/getopt_pp_standalone.h"

#include "BrainThread.h"
#include "Parser.h"
#include "Debuger.h"
#include "LogStream.h"
#include "BrainThreadExceptions.h"

#include "BrainHelp.h"

//sekcje krytyczne
CRITICAL_SECTION code_critical_section;
CRITICAL_SECTION cout_critical_section;
CRITICAL_SECTION pm_critical_section;
CRITICAL_SECTION heap_critical_section;

//flagi
bool OP_debug, OP_repair, OP_execute, OP_nopause;
MessageLog::MessageLevel OP_message;
LogStream::stream_type OP_log;
std::string OP_source = "";
Parser::code_lang OP_language;

MemoryTape<char>::mem_option OP_mem_behavior;
MemoryTape<char>::eof_option OP_eof_behavior; //reakcja na EOF z wejœcia
BrainThreadProcess<char>::res_context OP_resource_context; //wspo³dzielenie zasobów przez w¹tki
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

//domyœlan wartoœæ
const unsigned int def_mem_size = 30000;


//inicjator argumentow
void InitArguments(GetOpt::GetOpt_pp &ops);

//Czy program zosta³ uruchomiony z cmd.exe?
bool RanFromConsole();

//£atwe wrappery
bool RunParseArguments(GetOpt::GetOpt_pp &ops);
bool RunParserAndDebug(CodeTape &codetape);
void RunProgram(CodeTape &codetape);
template < typename T > void RunProgram(BrainThread<T> &brain_main_thread, CodeTape &codetape);

#define TYPEMOI unsigned short

int main(int argc, char* argv[])
{
	//inicjalizacja sekcji krytycznych
	InitializeCriticalSection(&code_critical_section);
	InitializeCriticalSection(&cout_critical_section);
	InitializeCriticalSection(&pm_critical_section);
	InitializeCriticalSection(&heap_critical_section);

	//
	CodeTape codetape;

	//parsowanie opcji + deklaracje potrzebnych flag
	GetOpt::GetOpt_pp ops(argc, argv);
	ops.exceptions(std::ios::failbit | std::ios::eofbit);

	if (ops >> GetOpt::OptionPresent('h',"help"))
	{
		std::string help_opt;
		ops >>  GetOpt::Option('h',"help", help_opt);
		ShowHelp(help_opt);
	}
	else if(argc > 1 && RunParseArguments(ops) == true)
	{
		if(RunParserAndDebug(codetape) == true && OP_execute == true)
		{
			RunProgram(codetape);
		}

		MessageLog::GetInstance().GetMessages();
	}
	else
	{
		//ShowUsage();
		ShowHelp("e");
	}
	
	//pa2.Parse(",[#,]$[.$]\0");
		//pa2.Parse("{[->-[-]<<+/>M!]}\\+++++++++++++++++++++++++.");
		//pa2.Parse(";[:[-]++++++++++.;]");
		//pa2.Parse("{[<[+]]+++++++++++++++++++++++++++++++++[-]+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++}.\0");
		//pa2.Parse("{[-<]}+++[>+++++<-]>[>+>+++>+>++>+++++>++<[++<]>---]>->-.[>++>+<<--]>--.--.+.>>>++.<<.<------.+.+++++.>>-.<++++.<--.>>>.<<---.<.-->-.>+.[+++++.---<]>>[.--->]<<.<+.++.++>+++[.<][.]<++.\0");
	
	

	if(OP_nopause == false)
		system("pause");
	
	DeleteCriticalSection(&code_critical_section);
	DeleteCriticalSection(&cout_critical_section);
	DeleteCriticalSection(&pm_critical_section);
	DeleteCriticalSection(&heap_critical_section);

	return 0;
}

bool RunParseArguments(GetOpt::GetOpt_pp &ops)
{
	bool ok = true;

	try
	{
		InitArguments(ops);
	}
	catch(GetOpt::TooManyArgumentsEx ex)
	{
		MessageLog::GetInstance().AddMessage("Argument Init Error: Too many arguments");
		ok = false;
	}
	catch(GetOpt::GetOptEx ex)
    {
        MessageLog::GetInstance().AddMessage("Argument Init Error: Error in arguments: " + std::string(ex.what()));
		ok = false;
    }
	catch(BrainThreadInvalidOptionException ex)
	{
		MessageLog::GetInstance().AddMessage("Argument Init Error: Invalid argument: " + std::string(ex.what()));
		ok = false;
	}
	catch(...)
	{
		MessageLog::GetInstance().AddMessage("Argument Init Error: General error");
		ok = false;
	}

	if(ok == true)
		MessageLog::GetInstance().AddInfo("Argument Init Ok");

	return ok;
}

bool RunParserAndDebug(CodeTape &codetape)
{
	bool ok = true;
	try
	{
		Parser parser(OP_language, &MessageLog::GetInstance(), OP_debug);

		if(OP_sourcetype == stInput)
		{
			parser.Parse(OP_source.c_str());
		}
		else if(OP_sourcetype == stFile)
		{
			std::ifstream in(OP_source);
			in.open(OP_source);
			parser.Parse(in);
			in.close();
		}

		if(parser.isCodeValid()) //kod wygl¹da w porz¹dku
		{
			MessageLog::GetInstance().AddInfo("Parse ends succesfully");
		
			if(OP_debug)
			{
				Debuger debuger(parser.GetCode(), sizeof(TYPEMOI), OP_repair);
				debuger.Debug();

				if(debuger.isCodeValid())
				{
					MessageLog::GetInstance().AddInfo("Debug ends succesfully");
					if(OP_repair == true)
						MessageLog::GetInstance().AddInfo("Repair ends succesfully");
				}
				else
				{
					MessageLog::GetInstance().AddMessage("Debug fails");
					ok = false;
				}
			}		
		}
		else
		{
			MessageLog::GetInstance().AddMessage("Parse Error: There are errors in your code");
			ok = false;
		}

		if(ok == true)
			parser.GetCode(codetape);
	}
	catch(std::exception e)
	{
		MessageLog::GetInstance().AddMessage("Parse&Debug Error:");
		ok = false;
	}

	return ok;
}

void RunProgram(CodeTape &codetape)
{
	switch(OP_cellsize)
	{
		case cs8:
		{
			BrainThread<char> brain;
			RunProgram(brain, codetape);
		}
		break;
		case cs16:
		{
			BrainThread<short> brain;
			RunProgram(brain, codetape);
		}
		break;
		case cs32:
		{
			BrainThread<int> brain;
			RunProgram(brain, codetape);
		}
		break;
		case csu8:
		{
			BrainThread<unsigned char> brain;
			RunProgram(brain, codetape);
		}
		break;
		case csu16:
		{
			BrainThread<unsigned short> brain;
			RunProgram(brain, codetape);
		}
		break;
		case csu32:
		{
			BrainThread<unsigned int> brain;
			RunProgram(brain, codetape);
		}
		break;
		default:
		{
			BrainThread<char> brain;
			RunProgram(brain, codetape);
		}
	}
}

template < typename T > 
void RunProgram(BrainThread<T> &brain_main_thread, CodeTape &codetape)
{
	brain_main_thread.mem_size = OP_mem_size;

	brain_main_thread.mem_behavior      = static_cast< MemoryTape<T>::mem_option >(OP_mem_behavior);
	brain_main_thread.eof_behavior      = static_cast< MemoryTape<T>::eof_option >(OP_eof_behavior);
	brain_main_thread.resource_context  = static_cast< BrainThreadProcess<T>::res_context >(OP_resource_context);
	
	brain_main_thread.Run(&codetape);
	brain_main_thread.WaitForPendingThreads();

	MessageLog::GetInstance().AddInfo("Execution end");
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
	OP_language = Parser::clBrainThread;

	// --sharedmemory
	if (ops >> GetOpt::OptionPresent("sharedmemory"))
	{
       OP_resource_context = BrainThreadProcess<char>::rcShared;
	}
	else OP_resource_context = BrainThreadProcess<char>::rcIndependent;

	//opcje œrodowiska

	//debug, repair & execute
	OP_debug = (ops >> GetOpt::OptionPresent('d', "debug"));
	OP_repair = (ops >> GetOpt::OptionPresent('r', "repair")); //niekoniecznie chce, aby debug naprawia³
	OP_execute = (ops >> GetOpt::OptionPresent('x', "execute"));  //niekoniecznie chce, aby po debugu uruchamia³
	
	if(OP_repair)//aby by³ repair, musi byc debug
		OP_debug = true;
	if(!OP_debug)//nie debugujesz? musi byc execute
		OP_execute = true;

	//--message[all|important|none]
	//--wall
	//--silent
	if (ops >> GetOpt::OptionPresent("messages") || ops >> GetOpt::OptionPresent("wall") || ops >> GetOpt::OptionPresent("silent"))
	{
        if(ops >> GetOpt::OptionPresent("messages"))
		{
			ops >>  GetOpt::Option("messages", op_arg);
			if(op_arg == "all")
				OP_message = MessageLog::mlAll;
			else if(op_arg == "important")
				OP_message = MessageLog::mlImportant;
			else if(op_arg == "none")
				OP_message = MessageLog::mlNone;
			else 
				throw BrainThreadInvalidOptionException("MESSAGES", op_arg);
		}

		if (ops >> GetOpt::OptionPresent("wall"))
			OP_message = MessageLog::mlImportant;

		if (ops >> GetOpt::OptionPresent("silent"))
			OP_message = MessageLog::mlNone;
	}
	OP_message = MessageLog::mlImportant;
	MessageLog::GetInstance().SetMessageLevel(OP_message);
	
	// --log [none|console|filename]
	if (ops >> GetOpt::OptionPresent("log"))
	{
		ops >>  GetOpt::Option("log", op_arg);

		if(op_arg.find(".") != std::string::npos) //rozpoznajemy ¿e wpisano plik
			OP_log = LogStream::lsFile;
		else if(op_arg == "none")
			OP_log = LogStream::lsNone;
		else if(op_arg == "console")
			OP_log = LogStream::lsConsole;
		else 
			throw BrainThreadInvalidOptionException("LANGUAGE", op_arg);

		LogStream::GetInstance().OpenStream(OP_log, op_arg);
	}
	else
	{
		LogStream::GetInstance().OpenStream(OP_log,"bt_log.txt");
	}

	//-s --sourcefile [filename]
	//-i --input [code]
	//--sourcecode [code]
	if (ops >> GetOpt::OptionPresent('s',"sourcefile"))
	{
		ops >>  GetOpt::Option('s',"sourcefile", op_arg);

		if(op_arg.find(".") != std::string::npos) //rozpoznajemy ¿e wpisano plik
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

		if(op_arg.find(".") != std::string::npos) //rozpoznajemy ¿e wpisano plik
		{
			OP_sourcetype = stInput;
			OP_source = op_arg;
		}
	}

	//--strict //super zgodne ustawienie opcji
	if(ops >> GetOpt::OptionPresent("strict"))
	{
		switch(OP_language)
		{
			case Parser::clBrainThread:
			case Parser::clBrainFork:
				OP_eof_behavior = MemoryTape<char>::eoMinusOne; //?
				OP_resource_context = BrainThreadProcess<char>::rcIndependent;
			break;

			case Parser::clPBrain:
			case Parser::clBrainFuck:
			default:
				OP_eof_behavior = MemoryTape<char>::eoUnchanged;
		}

		OP_mem_size = def_mem_size;
		OP_debug = false;
		OP_execute = true;

	}

	//--nopause //czy chcesz zatrzymaæ program po wykonaniu
	if(ops >> GetOpt::OptionPresent("nopause") == false)
	{
		OP_nopause = RanFromConsole(); //nie ma opcji - pozwól samemu wykminic czy jest potrzeba
	}
	else OP_nopause = true;

	//reszta opcji, w³aœciwie spodziewany sie jednej tylko
	if(ops.options_remain())
	{
		ops >> GetOpt::GlobalOption(op_args);

		if(op_args.size() != 1)
				throw GetOpt::TooManyArgumentsEx();
		else 
		{
			if(op_args[0].find_first_of(",[]<>+-()") == std::string::npos &&
			   op_args[0].find(".") != std::string::npos ) //rozpoznajemy ¿e wpisano plik
			{
				OP_sourcetype = stFile;
				OP_source = op_arg;
			}
			else
			{
				OP_sourcetype = stInput;
				OP_source = op_arg;
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

