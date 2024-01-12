// Defines the entry point for the console application.
//
#include <memory>
#include <chrono>
#include <sstream>
#include <algorithm>

#include "Settings.h"
#include "Interpreter.h"
#include "Parser.h"
#include "CodeAnalyser.h"
#include "ProcessMonitor.h"
#include "BrainThreadExceptions.h"
#include "BrainHelp.h"

using namespace BT;

//critical sections
CRITICAL_SECTION cout_critical_section;
CRITICAL_SECTION cin_critical_section;
CRITICAL_SECTION pm_critical_section;
CRITICAL_SECTION heap_critical_section;

//Reaction to ctrl+break
bool CtrlHandler(DWORD fdwCtrlType);

//Main methods
void InteractiveMode();
void Execute(const Settings& flags);

//Program methods
ParserBase ParseCode(const std::string& code, const Settings& flags);
void RunAnalyser(ParserBase& parser, const Settings& flags);
void RunProgram(const CodeTape& code, const Settings& flags);

int main(int argc, char* argv[])
{
	//settings
	Settings settings;

	InitializeCriticalSection(&cout_critical_section);
	InitializeCriticalSection(&cin_critical_section);
	InitializeCriticalSection(&pm_critical_section);
	InitializeCriticalSection(&heap_critical_section);

	//ctrl+break termination handler
	SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, true);

	//parse arguments
	GetOpt::GetOpt_pp ops(argc, argv);
	ops.exceptions(std::ios::failbit | std::ios::eofbit);

	if (ops >> GetOpt::OptionPresent('h',"help"))
	{
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
		ShowInfo();
	}
	else if(argc > 1)
	{
		if(settings.InitFromArguments(ops))
		{
			if(settings.OP_message == MessageLog::MessageLevel::mlAll){
				PrintBrainThreadInfo();
			}  
			
			Execute(settings);
		}
		else ShowUsage();

		MessageLog::Instance().PrintMessages();
	}
	else
	{
		PrintBrainThreadInfoEx();
		InteractiveMode();
		settings.OP_nopause = true;
	}
	
	if(settings.OP_nopause == false)
		system("pause");
	
	DeleteCriticalSection(&cout_critical_section);
	DeleteCriticalSection(&cin_critical_section);
	DeleteCriticalSection(&pm_critical_section);
	DeleteCriticalSection(&heap_critical_section);

	return 0;
}

void Execute(const Settings& flags) {

	MessageLog::Instance().SetMessageLevel(flags.OP_message);
	ParserBase parser = ParseCode(flags.OP_source_code, flags);

	if (!parser.IsSyntaxValid())
		return;

	if (flags.OP_analyse || flags.OP_optimize) {
		RunAnalyser(parser, flags);
	}
	if (flags.OP_execute) {
		RunProgram(parser.GetInstructions(), flags);
	}
}

ParserBase ParseCode(const std::string& code, const Settings& flags)
{
	switch (flags.OP_language)
	{
		case CodeLang::clBrainThread:
		{
			if (flags.OP_analyse) return Parser<CodeLang::clBrainThread, 0>(code);
			else if (flags.OP_optimize) return Parser<CodeLang::clBrainThread, 2>(code);
			else return Parser<CodeLang::clBrainThread, 1>(code);
		}
		break;
		case CodeLang::clPBrain:
		{
			if (flags.OP_analyse) return Parser<CodeLang::clPBrain, 0>(code);
			else if (flags.OP_optimize) return Parser<CodeLang::clPBrain, 2>(code);
			else return Parser<CodeLang::clPBrain, 1>(code);
		}
		break;
		case CodeLang::clBrainFork:
		{
			if (flags.OP_analyse) return Parser<CodeLang::clBrainFork, 0>(code);
			else if (flags.OP_optimize) return Parser<CodeLang::clBrainFork, 2>(code);
			else return Parser<CodeLang::clBrainFork, 1>(code);
		}
		case CodeLang::clBrainFuck:
		default:
		{
			if (flags.OP_analyse) return Parser<CodeLang::clBrainFuck, 0>(code);
			else if (flags.OP_optimize) return Parser<CodeLang::clBrainFuck, 2>(code);
			else return Parser<CodeLang::clBrainFuck, 1>(code);
		}
		break;
	}
}

std::unique_ptr<InterpreterBase> ProduceInterpreter(const Settings& flags)
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

void RunAnalyser(ParserBase& parser, const Settings& flags)
{
	try
	{
		if (parser.IsSyntaxValid()) //syntax looks fine
		{
			MessageLog::Instance().AddInfo("Parser: Syntax is valid");
			
			if (flags.OP_analyse)
			{
				CodeAnalyser analyser(parser);
				flags.OP_optimize ? analyser.Repair() : analyser.Analyse();

				if (analyser.isCodeValid())
				{
					if (analyser.RepairedSomething() == true)
						MessageLog::Instance().AddInfo("Some bugs have been successfully fixed");

					MessageLog::Instance().AddInfo("Code Analyser: Code is sane");
				}
				else
				{
					MessageLog::Instance().AddInfo("Code Analyser: Code has warnings");
				}
			}
		}
		else
		{
			MessageLog::Instance().AddMessage("Parser: Invalid syntax");
		}

	}
	catch (...)
	{
		MessageLog::Instance().AddMessage(MessageLog::ErrCode::ecUnknownError, "In function RunParserAndDebug()");
	}
}

void RunProgram(const CodeTape& code, const Settings& flags)
{
	auto start = std::chrono::system_clock::now();

	auto interpreter = ProduceInterpreter(flags);
	interpreter->Run(code);
	ProcessMonitor::Instance().WaitForWorkingProcesses();
	
	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast <std::chrono::milliseconds> (end - start).count();

    MessageLog::Instance().AddInfo("Execution completed in " + std::to_string(elapsed) + " miliseconds");
}

void InteractiveMode() {
	Settings s;

	s.OP_analyse = false;

	s.OP_message = MessageLog::MessageLevel::mlAll;
	//handy reference to source code
	std::string& input = s.OP_source_code;

	std::cout << "Interactive Mode: Enter your code, type 'exit' to break." << std::endl;

	while (true) {
		std::cout << "\n>>> " << std::flush;
		std::cin >> input;
		if (std::cin.fail())
		{
			std::cin.clear();
			std::cin.ignore(UINT_MAX, '\n');
			std::cout << "Input failed. Please try again." << std::endl;
		}

		std::transform(input.begin(), input.end(), input.begin(),
			[](unsigned char c) { return std::tolower(c); });
		
		if (input == "exit") {
			break;
		}
		else {
			MessageLog::Instance().ClearMessages();
			Execute(s);
			MessageLog::Instance().PrintMessages();
		}
	}
}

bool CtrlHandler(DWORD fdwCtrlType) 
{ 
  if(fdwCtrlType == CTRL_BREAK_EVENT ) 
  { 
	  MessageLog::Instance().AddMessage("Execution interrupted by user");
	  MessageLog::Instance().PrintMessages();
	  return true; //??
  } 
  return false; //pass all normally
} 

