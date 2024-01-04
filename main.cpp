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
#include "BrainThreadExceptions.h"
#include "BrainHelp.h"

using namespace BT;

//critical sections
CRITICAL_SECTION cout_critical_section;
CRITICAL_SECTION pm_critical_section;
CRITICAL_SECTION heap_critical_section;

//flags
BTFlags flags;
const unsigned int def_mem_size = 30000;

bool InitArguments(GetOpt::GetOpt_pp &ops);

//Czy program zosta³ uruchomiony z cmd.exe?
bool RanFromConsole();
//Rekacja na prwerwanie programu ctrl+break
bool CtrlHandler(DWORD fdwCtrlType);

//wrappers
std::string GetCode();
ParserBase ParseCode(std::string code);
void RunAnalyser(const ParserBase& parser);
void RunProgram(const CodeTape &code);

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
		flags.OP_nopause = RanFromConsole();
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
		flags.OP_nopause = RanFromConsole();
		ShowInfo();
	}
	else if(argc > 1)
	{
		if(InitArguments(ops)) 
		{
			if(flags.OP_message != MessageLog::mlNone){
				PrintBrainThreadInfo();
			}  
			
			ParserBase parser = ParseCode(GetCode());

			if (parser.IsSyntaxValid() && (flags.OP_debug || flags.OP_optimize)){
				RunAnalyser(parser);
			}

			if(parser.IsSyntaxValid() && flags.OP_execute){
				RunProgram(parser.GetInstructions());
			}
		}

		MessageLog::Instance().GetMessages();
	}
	else
	{
		ShowUsage();
	}
	
	if(flags.OP_nopause == false)
		system("pause");
	
	DeleteCriticalSection(&cout_critical_section);
	DeleteCriticalSection(&pm_critical_section);
	DeleteCriticalSection(&heap_critical_section);

	return 0;
}

std::string GetCode()
{
	if(flags.OP_sourcetype == sourcetype_option::stInput)
	{
		return std::move(flags.OP_source);
	}
	else if(flags.OP_sourcetype == sourcetype_option::stFile)
	{
		try {
			std::ifstream in(flags.OP_source);
			if (in.fail())
				throw std::ios_base::failure("File read error");

			std::string buffer;
			std::copy(std::istream_iterator<char>(in), std::istream_iterator<char>(), std::back_inserter(buffer));

			in.close();
			return std::move(buffer);
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

ParserBase ParseCode(std::string code)
{
	switch (flags.OP_language)
	{
		case CodeLang::clBrainThread:
		{
			if (flags.OP_debug) return Parser<CodeLang::clBrainThread, 0>(code);
			else if (flags.OP_optimize) return Parser<CodeLang::clBrainThread, 2>(code);
			else return Parser<CodeLang::clBrainThread, 1>(code);
		}
		break;
		case CodeLang::clPBrain:
		{
			if (flags.OP_debug) return Parser<CodeLang::clPBrain, 0>(code);
			else if (flags.OP_optimize) return Parser<CodeLang::clPBrain, 2>(code);
			else return Parser<CodeLang::clPBrain, 1>(code);
		}
		break;
		case CodeLang::clBrainFork:
		{
			if (flags.OP_debug) return Parser<CodeLang::clBrainFork, 0>(code);
			else if (flags.OP_optimize) return Parser<CodeLang::clBrainFork, 2>(code);
			else return Parser<CodeLang::clBrainFork, 1>(code);
		}
		case CodeLang::clBrainFuck:
		default:
		{
			if (flags.OP_debug) return Parser<CodeLang::clBrainFuck, 0>(code);
			else if (flags.OP_optimize) return Parser<CodeLang::clBrainFuck, 2>(code);
			else return Parser<CodeLang::clBrainFuck, 1>(code);
		}
		break;
	}
}

std::unique_ptr<InterpreterBase> ProduceInterpreter()
{
	switch (flags.OP_cellsize)
	{
		case cellsize_option::cs16: return std::make_unique<Interpreter<short>>(flags.OP_mem_behavior, flags.OP_eof_behavior, flags.OP_mem_size);
		break;
		case cellsize_option::cs32: return std::make_unique <Interpreter<int>>(flags.OP_mem_behavior, flags.OP_eof_behavior, flags.OP_mem_size);
		break;
		case  cellsize_option::csu8: return std::make_unique<Interpreter<unsigned char>>(flags.OP_mem_behavior, flags.OP_eof_behavior, flags.OP_mem_size);
		break;
		case  cellsize_option::csu16: return std::make_unique<Interpreter<unsigned short>>(flags.OP_mem_behavior, flags.OP_eof_behavior, flags.OP_mem_size);
		break;
		case  cellsize_option::csu32: return std::make_unique<Interpreter<unsigned int>>(flags.OP_mem_behavior, flags.OP_eof_behavior, flags.OP_mem_size);
		break;
		case cellsize_option::cs8: 
		default: return std::make_unique<Interpreter<char>>(flags.OP_mem_behavior, flags.OP_eof_behavior, flags.OP_mem_size);
	}
}

void RunAnalyser(const ParserBase& parser)
{
	try
	{
		if (parser.IsSyntaxValid()) //syntax looks fine
		{
			MessageLog::Instance().AddInfo("Code is valid");
			/*
			if (flags.OP_debug)
			{
				CodeAnalyser analyser(parser.GetInstructions());
				flags.OP_optimize ? analyser.Repair() : analyser.Analyse();

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
			}*/
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

void RunProgram(const CodeTape& code)
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
				flags.OP_eof_behavior = eof_option::eoZero;
			else if(op_arg == "255")
				flags.OP_eof_behavior = eof_option::eoMinusOne;
			else if(op_arg == "nochange")
				flags.OP_eof_behavior = eof_option::eoUnchanged;
			else 
				throw BrainThreadInvalidOptionException("EOF", op_arg);
		}
		else flags.OP_eof_behavior = eof_option::eoZero;
	
		// -c --cellsize [8|16|32|u8|u16|u32]
		if (ops >> GetOpt::OptionPresent('c',"cellsize"))
		{
			ops >>  GetOpt::Option('c',"cellsize", op_arg);
			if(op_arg == "8")
				flags.OP_cellsize = cellsize_option::cs8;
			else if(op_arg == "16")
				flags.OP_cellsize = cellsize_option::cs16;
			else if(op_arg == "32")
				flags.OP_cellsize = cellsize_option::cs32;
			else if(op_arg == "u8")
				flags.OP_cellsize = cellsize_option::csu8;
			else if(op_arg == "u16")
				flags.OP_cellsize = cellsize_option::csu16;
			else if(op_arg == "u32")
				flags.OP_cellsize = cellsize_option::csu32;
			else 
				throw BrainThreadInvalidOptionException("CELLSIZE", op_arg);
		}
		else flags.OP_cellsize = cellsize_option::cs8;

		// -m --memorysize <1,2^32>
		if (ops >> GetOpt::OptionPresent('m',"memorysize"))
		{
			ops >>  GetOpt::Option('m', "memorysize", op_arg);
			op_arg_i = atoi(op_arg.c_str());

			if(op_arg_i < 1 || op_arg_i > UINT_MAX )
				throw BrainThreadInvalidOptionException("MEMORYSIZE", op_arg);
			else 
				flags.OP_mem_size = (unsigned int)op_arg_i;
		}
		else flags.OP_mem_size = def_mem_size;

		// -b --memorybehavior [constant|tapeloop|dynamic]
		if (ops >> GetOpt::OptionPresent('b',"memorybehavior"))
		{
			ops >>  GetOpt::Option('b', "memorybehavior", op_arg);
			if(op_arg == "constant")
				flags.OP_mem_behavior = mem_option::moLimited;
			else if(op_arg == "tapeloop")
				flags.OP_mem_behavior = mem_option::moContinuousTape;
			else if(op_arg == "dynamic")
				flags.OP_mem_behavior = mem_option::moDynamic;
			else 
				throw BrainThreadInvalidOptionException("MEMORYBEHAVIOR", op_arg);
		}
		else flags.OP_mem_behavior = mem_option::moLimited;
	
		// -l --language [bt|b|bf|pb|brainthread|brainfuck|brainfork|pbrain|auto]
		if (ops >> GetOpt::OptionPresent('l',"language"))
		{
			ops >>  GetOpt::Option('l', "language", op_arg);
			if(op_arg == "bt" || op_arg == "brainthread")
				flags.OP_language = CodeLang::clBrainThread;
			else if(op_arg == "b" || op_arg == "brainfuck")
				flags.OP_language = CodeLang::clBrainFuck;
			else if(op_arg == "bf" || op_arg == "brainfork")
				flags.OP_language = CodeLang::clBrainFork;
			else if(op_arg == "pb" || op_arg == "pbrain")
				flags.OP_language = CodeLang::clPBrain;
			else 
				throw BrainThreadInvalidOptionException("LANGUAGE", op_arg);
		}

		//opcje œrodowiska

		//debug, repair & execute
		flags.OP_debug = (ops >> GetOpt::OptionPresent('a', "analyse"));
	
		flags.OP_optimize = (ops >> GetOpt::OptionPresent('o', "optimize")); //todo o2 o3
		flags.OP_repair = flags.OP_optimize || (ops >> GetOpt::OptionPresent('r', "repair")); //niekoniecznie chce, aby debug naprawia³
		flags.OP_execute = (ops >> GetOpt::OptionPresent('x', "execute"));  //niekoniecznie chce, aby po debugu uruchamia³
	
		if(flags.OP_optimize || flags.OP_repair)//aby by³ repair, musi byc debug
			flags.OP_debug = true;
		if(flags.OP_debug == false)//nie debugujesz? musi byc execute
			flags.OP_execute = true;

		//--verbose[all|important|none]
		//--wall
		//--silent
		if (ops >> GetOpt::OptionPresent("verbose") ||  ops >> GetOpt::OptionPresent("silent"))
		{
			if(ops >> GetOpt::OptionPresent("verbose"))
			{
				ops >>  GetOpt::Option("verbose", op_arg);
				if(op_arg == "all")
					flags.OP_message = MessageLog::mlAll;
				else if(op_arg == "important")
					flags.OP_message = MessageLog::mlImportant;
				else if(op_arg == "none")
					flags.OP_message = MessageLog::mlNone;
				else 
					throw BrainThreadInvalidOptionException("VERBOSE", op_arg);
			}

			if (ops >> GetOpt::OptionPresent("silent"))
			{
				flags.OP_message = MessageLog::mlNone;
			}
		}
		else
			flags.OP_message = MessageLog::mlImportant;

		MessageLog::Instance().SetMessageLevel(flags.OP_message);
	
		// --log [none|console|filename]
		if (ops >> GetOpt::OptionPresent("log"))
		{
			ops >>  GetOpt::Option("log", op_arg);

			if(op_arg.find(".") != std::string::npos) //rozpoznajemy ¿e wpisano plik
				flags.OP_log = DebugLogStream::lsFile;
			else if(op_arg == "none")
				flags.OP_log = DebugLogStream::lsNone;
			else if(op_arg == "console")
				flags.OP_log = DebugLogStream::lsConsole;
			else 
				throw BrainThreadInvalidOptionException("LOG", op_arg);

			DebugLogStream::Instance().Init(flags.OP_log, op_arg);
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
				flags.OP_sourcetype = sourcetype_option::stFile;
				flags.OP_source = op_arg;
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
		
			flags.OP_sourcetype = sourcetype_option::stInput;
			flags.OP_source = op_arg;
		}

		//--strict //super zgodne ustawienie opcji
		if(ops >> GetOpt::OptionPresent("strict"))
		{
			switch(flags.OP_language)
			{
				case CodeLang::clBrainThread:
				case CodeLang::clBrainFork:
					flags.OP_eof_behavior = eof_option::eoMinusOne;
				break;

				case CodeLang::clPBrain:
				case CodeLang::clBrainFuck:
				default:
					flags.OP_eof_behavior = eof_option::eoUnchanged;
				
			}

			flags.OP_mem_size = def_mem_size;
			flags.OP_cellsize = cellsize_option::cs8;
			flags.OP_mem_behavior = mem_option::moLimited;
		}

		//--nopause //czy chcesz zatrzymaæ program po wykonaniu
		if(ops >> GetOpt::OptionPresent("nopause") == false)
		{
			flags.OP_nopause = RanFromConsole(); //nie ma opcji - pozwól samemu wykminic czy jest potrzeba
		}
		else flags.OP_nopause = true;

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
					flags.OP_sourcetype = sourcetype_option::stFile;
					flags.OP_source = op_args[0];
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
	  if(flags.OP_execute)
		  MessageLog::Instance().AddInfo("Execution interrupted by user");

	  MessageLog::Instance().GetMessages();
	  return true; //??
  } 
  return false; //pass all normally
} 

