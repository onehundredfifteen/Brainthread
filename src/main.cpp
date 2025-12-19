// Defines the entry point for the console application.
//
#include <sstream>
#include <algorithm>

#ifdef _WIN32
 #include <windows.h>
#endif

#include "BrainThread.h"
#include "BrainHelp.h"

void InteractiveMode();
#ifdef _WIN32
 //Reaction to ctrl+break
 bool CtrlHandler(DWORD fdwCtrlType);
#endif

int main(int argc, char* argv[])
{
	//settings
	BT::Settings settings;

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
			if(settings.OP_message == BT::MessageLog::MessageLevel::mlAll) {
				PrintBrainThreadInfo();
			}  		
			RunProgram(settings);
		}
		else ShowUsage(settings.PAR_exe_path);

		BT::MessageLog::Instance().PrintMessages();
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

void InteractiveMode() {
	BT::Settings s;
	s.OP_analyse = true;
	s.OP_message = BT::MessageLog::MessageLevel::mlAll;

	std::string input;

	std::cout << "Welcome in the interactive mode! Enter your code, [help], [info], [exit] to break." << std::endl;

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

		s.OP_source_code = input;
		std::transform(input.begin(), input.end(), input.begin(),
			[](unsigned char c) { return std::tolower(c); });
		
		if (input == "exit") break;
		else if (input == "info") ShowInfo();
		else if (input == "help" || input == "usage" || input == "?") {
			ShowUsage(s.PAR_exe_path);
		}
		else if (input.size() > 4 && input.substr(4) == "set ") {
			BT::Settings new_settings;
			if (new_settings.InitFromString(input.substr(4))) {
				s = new_settings;
				s.OP_analyse = true;
				s.OP_message = BT::MessageLog::MessageLevel::mlAll;
				std::cout << "> New settings applied" << std::endl;
			}
			else {
				std::cout << "> Cannot apply this setting" << std::endl;
				BT::MessageLog::Instance().PrintMessages();
				BT::MessageLog::Instance().ClearMessages();
			}
		}
		else {
			BT::MessageLog::Instance().ClearMessages();
			RunProgram(s);
			BT::MessageLog::Instance().PrintMessages();
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
