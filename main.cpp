// Defines the entry point for the console application.
//
#include <memory>
#include <chrono>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
 #include <windows.h>
#endif

#include "Settings.h"
#include "Interpreter.h"
#include "Parser.h"
#include "CodeAnalyser.h"
#include "BrainThreadExceptions.h"
#include "BrainHelp.h"

using namespace BT;

#ifdef _WIN32
 //Reaction to ctrl+break
 bool CtrlHandler(DWORD fdwCtrlType);
#endif

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

#ifdef _WIN32
	//ctrl+break termination handler
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, true);
#endif

	//parse arguments
	GetOpt::GetOpt_pp ops(argc, argv);
	ops.exceptions(std::ios::failbit | std::ios::eofbit);
	
	if (ops >> GetOpt::OptionPresent("info")) {
		ShowInfo();
	}
	else if (ops >> GetOpt::OptionPresent("help") ||
			 ops >> GetOpt::OptionPresent("usage")) {
		ShowUsage(std::string(argv[0]));
	}
	else if(argc > 1)
	{	
		if(settings.InitFromArguments(ops))
		{
			if(settings.OP_message == MessageLog::MessageLevel::mlAll) {
				PrintBrainThreadInfo();
			}  		
			Execute(settings);
		}
		else ShowUsage(settings.PAR_exe_path);

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

	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast <std::chrono::milliseconds> (end - start).count();

    MessageLog::Instance().AddInfo("Execution completed in " + std::to_string(elapsed) + " miliseconds");
}

void InteractiveMode() {
	Settings s;
	s.OP_analyse = true;
	s.OP_message = MessageLog::MessageLevel::mlAll;
	//handy reference to source code
	std::string& input = s.OP_source_code;

	std::cout << "Interactive Mode: Enter your code, type 'exit' to break." << std::endl;

	while (true) {
		std::cout << "\n>>> " << std::flush;
		std::getline(std::cin, input);
		if (std::cin.fail())
		{
			std::cin.clear();
			std::cin.ignore(UINT_MAX, '\n');
			std::cout << "Input failed. Please try again." << std::endl;
			continue;
		}

		std::transform(input.begin(), input.end(), input.begin(),
			[](unsigned char c) { return std::tolower(c); });
		
		if (input == "exit") break;
		else if (input == "info") ShowInfo();
		else if (input == "help" || input == "usage" || input == "?") {
			ShowUsage(s.PAR_exe_path);
		}
		else if (input._Starts_with("set ")) {
			Settings new_settings;
			if (new_settings.InitFromString(input.substr(4))) {
				s = new_settings;
				s.OP_analyse = true;
				s.OP_message = MessageLog::MessageLevel::mlAll;
				std::cout << " New settings applied" << std::endl;
			}
			else {
				MessageLog::Instance().PrintMessages();
				MessageLog::Instance().ClearMessages();
			}
		}
		else {
			MessageLog::Instance().ClearMessages();
			Execute(s);
			MessageLog::Instance().PrintMessages();
		}
	}
}

#ifdef _WIN32
bool CtrlHandler(DWORD fdwCtrlType)
{
	if (fdwCtrlType == CTRL_BREAK_EVENT)
	{
		MessageLog::Instance().AddInfo("Execution interrupted by user");
		MessageLog::Instance().PrintMessages();
		return true; //??
	}
	return false; //pass all normally
}
#endif
