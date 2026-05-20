#include "infoAndHelp.h"

void PrintBrainThreadInfo(std::ostream& out)
{
	out << "BrainThread Interpreter " << BT_VERSION <<
				 " (c) by onehundredfifteen 2014-" << BT_DATE << std::endl;
}

void PrintBrainThreadInfoEx(std::ostream& out)
{
	PrintBrainThreadInfo(out);
	out << "https://github.com/onehundredfifteen/Brainthread\n"
				 "Supports BrainThread, BrainFuck, pBrain and BrainFork\n" << std::endl;
}

void ShowUsage(std::ostream& out, const std::string& exe_path)
{
	PrintBrainThreadInfoEx(out);
	std::size_t found = exe_path.rfind('\\');
	std::string exe_name = (found == std::string::npos) ? exe_path : exe_path.substr(found + 1);
	out << "\t++ From command line ++\n"
	#ifdef _WIN32
		<< "Run your code: \t" << exe_name << " [\"sourcefile.ext\"|\"sourcecode\"]\n"
	#else
		<< "Run your code: \t" << exe_name << " [-s \"sourcefile.ext\"|i- \"sourcecode\"]\n"
	#endif	
		<< "Run your code with options: \t" << exe_name << " [\"sourcefile.ext\"|\"sourcecode\"] [options]\n"
		<< "Show information about the Brainthread language: \t" << exe_name << " --info\n"
		<< "\n\t++ Main parameters ++\n"
		<< "-l --language [bt|b|bf|pb|brainthread|brainfuck|brainfork|pbrain] Default: brainthread\n"
		<< "-m --memorysize <1, 2^32> Default: 30000\n"
		<< "-c --cellsize [8|16|32|u8|u16|u32] Default: 8\n"
		<< "\n\t++ Interpreter options ++\n"
		<< "-a --analyze  \tDefault: flag is not set\n"
		<< "-o --optimize \tDefault: flag is not set\n"
		<< "-r --repair   \tDefault: flag is not set\n"
		<< "--nopause     \tDefault: flag is not set\n"
		<< "--verbose [all|important|none]\tDefault: important\n"
		<< "You can use these parameters in the interactive mode by typing 'set [params]'\n"
		<< "Other options can be found in the documentation"
		<< std::endl;
}

void ShowInfo(std::ostream& out)
{
	PrintBrainThreadInfo(out);

	out << "\n++ Quick dive into Brainthread language ++\n"
		<< "Brainthread is a derivative of Brainfuck. Supports functions (like pBrain), threads (like Brainfork) and stack. "
		<< "Each thread has it's own separate memory. Threads can use and communicate each other by the stack.\n"
		<< "\n++ Threading commands ++\n"
		<< " { - fork\t} - wait for children or terminate+detach if cell = 0\n"
		<< "\n++ Function commands ++\n"
		<< " ( - begin\t) - end of function definition\n"
		<< " * - invoke the function of which the identifier is equal to the current cell value.\n"
		<< "\n++ Stack commands ++\n"
		<< " ! - push\t^ - pop\t % - swap\n"
		<< " : - decimal write\t; - decimal read\n"
		<< "\n++ Additional 'Analyse mode' commands ++\n"
		<< " M, D, # - memory dumps (#-brainfuck only)\n"
		<< " F, E, S, T  - function/stack/thread dumps\n"
		<< "\n++ Interactive mode ++\n"
		<< " #numX - repeat instruction X num times i.e \"#21+\"\n"
		<< std::endl;
}