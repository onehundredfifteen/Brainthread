// Defines the entry point for the console application.
//
#include <memory>
#include <chrono>
#include <sstream>
#include <windows.h>
#include <process.h>


#pragma warning(push, 0)
#include "getoptpp/getopt_pp_standalone.h"
#pragma warning(pop)

#include "Enumdefs.h"
#include "Interpreter.h"
#include "Parser.h"
#include "CodeAnalyser.h"
#include "ProcessMonitor.h"
#include "DebugLogStream.h"
#include "BrainThreadExceptions.h"
#include "BrainHelp.h"

using namespace BT;

//critical sections
CRITICAL_SECTION cout_critical_section;
CRITICAL_SECTION pm_critical_section;
CRITICAL_SECTION heap_critical_section;

//flags
bool OP_debug, OP_repair, OP_optimize, OP_execute, OP_nopause;

MessageLog::MessageLevel OP_message;
DebugLogStream::stream_type OP_log;
std::string OP_source = "";
BT::CodeLang OP_language;

mem_option OP_mem_behavior;
eof_option OP_eof_behavior; //reakcja na EOF z wejœcia
unsigned int OP_mem_size;
cellsize_option OP_cellsize;
sourcetype_option OP_sourcetype;

const unsigned int def_mem_size = 30000;

//inicjator argumentow
bool InitArguments(GetOpt::GetOpt_pp &ops);

//Czy program zosta³ uruchomiony z cmd.exe?
bool RanFromConsole();
//Rekacja na prwerwanie programu ctrl+break
bool CtrlHandler(DWORD fdwCtrlType);

//wrappers
ParserBase ProduceParser();
void RunProgram(CodeTape code);

int main(int argc, char* argv[])
{
	InitializeCriticalSection(&cout_critical_section);
	InitializeCriticalSection(&pm_critical_section);
	InitializeCriticalSection(&heap_critical_section);

	//inicjalizacja handlera obs³uguj¹ego zamkniêcie programu sktórem ctrl+break
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
		if(InitArguments(ops)) 
		{
			if(OP_message != MessageLog::mlNone){
				PrintBrainThreadInfo();
			}  
			
			ParserBase parser = ProduceParser();
			//if (OP_debug){
			//}

			if(MessageLog::Instance().ErrorsCount() == 0 && OP_execute == true){
				RunProgram(parser.GetInstructions());
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

std::string GetCode()
{
	if(OP_sourcetype == sourcetype_option::stInput)
	{
		return OP_source;
	}
	else if(OP_sourcetype == sourcetype_option::stFile)
	{
		try {
			std::ifstream in(OP_source);
			if (in.fail())
				throw std::ios_base::failure("File read error");

			std::string buffer;
			std::copy(std::istream_iterator<char>(in), std::istream_iterator<char>(), std::back_inserter(buffer));

			in.close();
			return buffer;
		}
		catch (std::ios_base::failure& e)
		{
			MessageLog::Instance().AddMessage(MessageLog::ecFatalError, std::string(e.what()));
		}
		catch (...)
		{
			MessageLog::Instance().AddMessage(MessageLog::ecUnknownError, "In function RunParserAndDebug()");
		}
	}	
	return "";
}

void RunAnalyser(ParserBase& parser)
{
	try
	{
		if (true) //code looks fine
		{
			MessageLog::Instance().AddInfo("Code is valid");

			if (OP_debug)
			{
				CodeAnalyser analyser(parser.GetInstructions());
				OP_optimize ? analyser.Repair() : analyser.Analyse();

				if (analyser.isCodeValid())
				{
					if (analyser.RepairedSomething() == true)
						MessageLog::Instance().AddInfo("Some bugs have been successfully fixed");

					MessageLog::Instance().AddInfo("Code is sane");
				}
				else
				{
					MessageLog::Instance().AddMessage("Code has warnings");
				}
			}
		}
		else
		{
			MessageLog::Instance().AddMessage("Code has errors");
		}

	}
	catch (...)
	{
		MessageLog::Instance().AddMessage(MessageLog::ecUnknownError, "In function RunParserAndDebug()");
	}
}

ParserBase ProduceParser()
{
	switch (OP_language)
	{
		case CodeLang::clBrainThread:
		{
			if (OP_debug) return Parser<CodeLang::clBrainThread, 0>(GetCode());
			else if (OP_optimize) return Parser<CodeLang::clBrainThread, 2>(GetCode());
			else return Parser<CodeLang::clBrainThread, 1>(GetCode());
		}
		break;
		case CodeLang::clPBrain:
		{
			if (OP_debug) return Parser<CodeLang::clPBrain, 0>(GetCode());
			else if (OP_optimize) return Parser<CodeLang::clPBrain, 2>(GetCode());
			else return Parser<CodeLang::clPBrain, 1>(GetCode());
		}
		break;
		case CodeLang::clBrainFork:
		{
			if (OP_debug) return Parser<CodeLang::clBrainFork, 0>(GetCode());
			else if (OP_optimize) return Parser<CodeLang::clBrainFork, 2>(GetCode());
			else return Parser<CodeLang::clBrainFork, 1>(GetCode());
		}
		case CodeLang::clBrainFuck:
		default:
		{
			if (OP_debug) return Parser<CodeLang::clBrainFuck, 0>(GetCode());
			else if (OP_optimize) return Parser<CodeLang::clBrainFuck, 2>(GetCode());
			else return Parser<CodeLang::clBrainFuck, 1>(GetCode());
		}
		break;
	}
}

std::unique_ptr<InterpreterBase> ProduceInterpreter()
{
	switch (OP_cellsize)
	{
		case cellsize_option::cs16: return std::make_unique<Interpreter<short>>(OP_mem_behavior, OP_eof_behavior, OP_mem_size);
		break;
		case cellsize_option::cs32: return std::make_unique <Interpreter<int>>(OP_mem_behavior, OP_eof_behavior, OP_mem_size);
		break;
		case  cellsize_option::csu8: return std::make_unique<Interpreter<unsigned char>>(OP_mem_behavior, OP_eof_behavior, OP_mem_size);
		break;
		case  cellsize_option::csu16: return std::make_unique<Interpreter<unsigned short>>(OP_mem_behavior, OP_eof_behavior, OP_mem_size);
		break;
		case  cellsize_option::csu32: return std::make_unique<Interpreter<unsigned int>>(OP_mem_behavior, OP_eof_behavior, OP_mem_size);
		break;
		case cellsize_option::cs8: 
		default: return std::make_unique<Interpreter<char>>(OP_mem_behavior, OP_eof_behavior, OP_mem_size);
	}
}

void RunProgram(CodeTape code)
{
	auto start = std::chrono::system_clock::now();

	auto interpreter = ProduceInterpreter();
	interpreter->Run(code);
	ProcessMonitor::Instance().WaitForWorkingProcesses();
	
	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast <std::chrono::milliseconds> (end - start).count();

    MessageLog::Instance().AddInfo("Execution completed in " + std::to_string(elapsed) + " miliseconds");
}

bool InitArguments(GetOpt::GetOpt_pp &ops)
{
	try 
	{
		std::string op_arg;
		std::vector<std::string> op_args;
		unsigned long long op_arg_i;
	
		// -e --eof [0|255|nochange]
		if (ops >> GetOpt::OptionPresent('e',"eof"))
		{
			ops >>  GetOpt::Option('e', "eof", op_arg);
			if(op_arg == "0")
				OP_eof_behavior = eof_option::eoZero;
			else if(op_arg == "255")
				OP_eof_behavior = eof_option::eoMinusOne;
			else if(op_arg == "nochange")
				OP_eof_behavior = eof_option::eoUnchanged;
			else 
				throw BrainThreadInvalidOptionException("EOF", op_arg);
		}
		else OP_eof_behavior = eof_option::eoZero;
	
		// -c --cellsize [8|16|32|u8|u16|u32]
		if (ops >> GetOpt::OptionPresent('c',"cellsize"))
		{
			ops >>  GetOpt::Option('c',"cellsize", op_arg);
			if(op_arg == "8")
				OP_cellsize = cellsize_option::cs8;
			else if(op_arg == "16")
				OP_cellsize = cellsize_option::cs16;
			else if(op_arg == "32")
				OP_cellsize = cellsize_option::cs32;
			else if(op_arg == "u8")
				OP_cellsize = cellsize_option::csu8;
			else if(op_arg == "u16")
				OP_cellsize = cellsize_option::csu16;
			else if(op_arg == "u32")
				OP_cellsize = cellsize_option::csu32;
			else 
				throw BrainThreadInvalidOptionException("CELLSIZE", op_arg);
		}
		else OP_cellsize = cellsize_option::cs8;

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
				OP_mem_behavior = mem_option::moLimited;
			else if(op_arg == "tapeloop")
				OP_mem_behavior = mem_option::moContinuousTape;
			else if(op_arg == "dynamic")
				OP_mem_behavior = mem_option::moDynamic;
			else 
				throw BrainThreadInvalidOptionException("MEMORYBEHAVIOR", op_arg);
		}
		else OP_mem_behavior = mem_option::moLimited;
	
		// -l --language [bt|b|bf|pb|brainthread|brainfuck|brainfork|pbrain|auto]
		if (ops >> GetOpt::OptionPresent('l',"language"))
		{
			ops >>  GetOpt::Option('l', "language", op_arg);
			if(op_arg == "bt" || op_arg == "brainthread")
				OP_language = CodeLang::clBrainThread;
			else if(op_arg == "b" || op_arg == "brainfuck")
				OP_language = CodeLang::clBrainFuck;
			else if(op_arg == "bf" || op_arg == "brainfork")
				OP_language = CodeLang::clBrainFork;
			else if(op_arg == "pb" || op_arg == "pbrain")
				OP_language = CodeLang::clPBrain;
			else 
				throw BrainThreadInvalidOptionException("LANGUAGE", op_arg);
		}

		//opcje œrodowiska

		//debug, repair & execute
		OP_debug = (ops >> GetOpt::OptionPresent('a', "analyse"));
	
		OP_optimize = (ops >> GetOpt::OptionPresent('o', "optimize")); //todo o2 o3
		OP_repair = OP_optimize || (ops >> GetOpt::OptionPresent('r', "repair")); //niekoniecznie chce, aby debug naprawia³
		OP_execute = (ops >> GetOpt::OptionPresent('x', "execute"));  //niekoniecznie chce, aby po debugu uruchamia³
	
		if(OP_optimize || OP_repair)//aby by³ repair, musi byc debug
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

			if(op_arg.find(".") != std::string::npos) //rozpoznajemy ¿e wpisano plik
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

			if(op_arg.find(".") != std::string::npos) //rozpoznajemy ¿e wpisano plik
			{
				OP_sourcetype = sourcetype_option::stFile;
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
		
			OP_sourcetype = sourcetype_option::stInput;
			OP_source = op_arg;
		}

		//--strict //super zgodne ustawienie opcji
		if(ops >> GetOpt::OptionPresent("strict"))
		{
			switch(OP_language)
			{
				case CodeLang::clBrainThread:
				case CodeLang::clBrainFork:
					OP_eof_behavior = eof_option::eoMinusOne;
				break;

				case CodeLang::clPBrain:
				case CodeLang::clBrainFuck:
				default:
					OP_eof_behavior = eof_option::eoUnchanged;
				
			}

			OP_mem_size = def_mem_size;
			OP_cellsize = cellsize_option::cs8;
			OP_mem_behavior = mem_option::moLimited;
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

			if(op_args.size() > 1)
					throw GetOpt::TooManyArgumentsEx();
			else 
			{  
				if(GetFileAttributesA(op_args[0].c_str()) == INVALID_FILE_ATTRIBUTES) 
				{
					throw GetOpt::InvalidFormatEx();
				}
				else //rozpoznajemy ¿e wpisano plik
				{
					OP_sourcetype = sourcetype_option::stFile;
					OP_source = op_args[0];
				}
			}
		}
		return true;
	}
	catch (GetOpt::TooManyArgumentsEx ex)
	{
		MessageLog::Instance().AddMessage(MessageLog::ecArgumentError, "Too many arguments");
		return false;
	}
	catch (GetOpt::GetOptEx ex)
	{
		MessageLog::Instance().AddMessage(MessageLog::ecArgumentError, "Error while parsing arguments: " + std::string(ex.what()));
		return false;
	}
	catch (BrainThreadInvalidOptionException ex)
	{
		MessageLog::Instance().AddMessage(MessageLog::ecArgumentError, std::string(ex.what()));
		return false;
	}
	catch (...)
	{
		MessageLog::Instance().AddMessage(MessageLog::ecArgumentError, "Unknown error");
		return false;
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

