#include <windows.h>
#include "BrainHelp.h"


std::string GetExeName(void)
{
	char szPath[MAX_PATH];
    if( GetModuleFileNameA(NULL, szPath, MAX_PATH) == 0 )
    {
        return "brainthread.exe";
    }
	return std::string(szPath).substr( std::string(szPath).rfind('\\') + 1 );
}

void PrintBrainThreadInfo(void)
{
	std::cout << "BrainThread Interpreter " << BT_VERSION << " (" << __DATE__ << ")\n"
				 "(c) by onehundredfifteen 2014-2017\n" << std::endl;
}

void PrintBrainThreadInfoEx(void)
{
	std::cout << "BrainThread Interpreter " << BT_VERSION << " (" << __DATE__ << ")\n"
				 "(c) by onehundredfifteen 2014-2017\n"
				 "https://github.com/onehundredfifteen/Brainthread\n"
				 "Supports BrainThread, BrainFuck, pBrain and BrainFork\n" << std::endl;
}

void ShowUsage(void)
{
	PrintBrainThreadInfoEx();
	std::cout << "\n++ USAGE ++\n"
			  << "\t" << GetExeName() << " [\"sourcefile.ext\"|\"sourcecode\"]\n"
			  << "\t" << GetExeName() << " [\"sourcefile.ext\"|\"sourcecode\"] [options]\n"
			  << "\t" << GetExeName() << " --help\n"
			  << "\t" << GetExeName() << " --info\n"
			  << std::endl;
}

void ShowHelp(std::string help_opt)
{
	PrintBrainThreadInfoEx();
	if(help_opt == "language" || help_opt == "l")
	{
	  std::cout << "OPTION: CODE LANGUAGE\n"
		        << "\n"
			    << "Syntax: -l --language [bt|b|bf|pb|brainthread|brainfuck|brainfork|pbrain|auto]\n"
				<< "Default value: brainthread\n"
				<< "\n"
				<< "Specify the language used to interpret the code. "
				<< "Option \"bt\" is equal to \"brainthread\", option \"b\" is equal to \"brainfuck\" and so on. "
				<< "Option \"auto\" launches a code language recognition algorithm.";
	}
	else if(help_opt == "eof" || help_opt == "e")
	{
	  std::cout << "OPTION: END-OF-LINE BEHAVIOR\n"
		        << "\n"
			    << "Syntax: -e --eof [0|255|nochange]\n"
				<< "Default value: 0\n"
				<< "\n"
				<< "The behavior of the ',' command when an end-of-file condition has been encountered. "
				<< "Option \"0\" set the cell at the pointer to 0, \"255\" set it to the C constant EOF (0xFF), "
				<< "and \"nochange\" leaves the cell's value unchanged. When a --strict flag is present, this option "
				<< "will be set by default to '0' for Brainthread and to 'nochange' for other languages.";
	}
	else if(help_opt == "memorysize" || help_opt == "m")
	{
	  std::cout << "OPTION: TAPE, ARRAY or MEMORY SIZE\n"
		        << "\n"
			    << "Syntax: -m --memorysize <1,2^32>\n"
				<< "Default value: 30000\n"
				<< "\n"
				<< "Define the length of tape used to store variables. The real size of memory in bytes is a result "
				<< "of multiplying array length by single cell size. When a --strict flag is present, this option "
				<< "will be set by default to '30000' regardless of your choice.";
	}
	else if(help_opt == "memorybehavior" || help_opt == "b")
	{
	  std::cout << "OPTION: MEMORY BEHAVIOR\n"
		        << "\n"
			    << "Syntax: -b --memorybehavior [constant|tapeloop|dynamic]\n"
				<< "Default value: constant\n"
				<< "\n"
				<< "The memory tape can behave in different ways. Option \"constant\" is a classic approach of an array "
				<< "constant in size. Option \"dynamic\" allows tape size to change dynamically each time when the pointer moves outside the left bound of the array. "
				<< "While the size is smaller than 2 MB, memory will be doubled. Above that limit, a 100KB space will be added. "
				<< "Option \"tapeloop\" provides a particular behavior. With this setting, tape will be constant in size, but without any boundaries. That means when "
				<< "the pointer will reach leftmost cell and moves left, the tape going to rewind and set it's position to rightmost cell and vice versa. "
				<< "Thus there is no possibility to raise a Range Exception. When a --strict flag is present, this option "
				<< "will be set by default to 'constant' regardless of your choice.";
	}
	else if(help_opt == "cellsize" || help_opt == "c")
	{
	  std::cout << "OPTION: SINGLE CELL SIZE\n"
		        << "\n"
			    << "Syntax: -c --cellsize [8|16|32|u8|u16|u32]\n"
				<< "Default value: 8\n"
				<< "\n"
				<< "Specify the size of a single cell in memory tape: 8, 16 or 32 bits. Options with an 'u' prefix define a storage of unsigned values. "
				<< "Then the real size of memory in bytes is a result of multiplying array length by this size. "
				<< "With sizes larger than 8, cells can store larger values - up to 4294967296 with 'u32' option. "
				<< "However, with those options, loops like [+] are significantly slower and commands ',' and '.' still can't "
				<< "read/write values bigger than 255. When a --strict flag is present, this option "
				<< "will be set by default to 'u8' regardless of your choice.";
	}
	/*else if(help_opt == "sharedmemory")
	{
	  std::cout << "OPTION: SHARED MEMORY\n"
		        << "\n"
			    << "Syntax: --sharedmemory\n"
				<< "Default value: Flag is not set\n"
				<< "\n"
				<< "This option only applies to Brainthread programs.\n"
				<< "When this flag is set, each thread shares its local heap with other threads. "
				<< "Thus, there is two heaps (local and common) for all processes. When a --strict flag is present, this option "
				<< "will be not set by default, regardless of your choice.";
	}*/
	else if(help_opt == "strict")
	{
	  std::cout << "OPTION: STRICT\n"
		        << "\n"
			    << "Syntax: --strict\n"
				<< "Default value: Flag is not set\n"
				<< "\n"
				<< "This flag enforces settings of Urban M�ller's brainfuck compiler, or, for other dialects, "
				<< "default settings for maximum compatibility of executed programs.";
	}
	else if(help_opt == "analyse" || help_opt == "a")
	{
	  std::cout << "OPTION: CODE ANALYSIS\n"
		        << "\n"
			    << "Syntax: -a --analyse\n"
				<< "Default value: Flag is not set\n"
				<< "\n"
				<< "This flag launches a code analyser. This will look for code weaknesses. "
				<< "Some problems with code can be fixed. Only with this flag special debugger instructions will be executed - p.ex. memory dump. ";
	}
	else if(help_opt == "repair" || help_opt == "r")
	{
	  std::cout << "OPTION: REPAIR\n"
		        << "\n"
			    << "Syntax: -r --repeir\n"
				<< "Default value: Flag is not set\n"
				<< "\n"
				<< "When this flag is set, the debugger will try to fix some problems encountered during code analyse. "
				<< "Flag --debug is set by default.";
	}
	else if(help_opt == "execute" || help_opt == "x")
	{
	  std::cout << "OPTION: EXCEUTE\n"
		        << "\n"
			    << "Syntax: -x --execute\n"
				<< "Default value: Flag is set\n"
				<< "\n"
				<< "When this flag is set, interpreter will run program's code. "
				<< "When option --debug is not present, this flag is set by default.";
	}
	else if(help_opt == "sourcefile" || help_opt == "s")
	{
	  std::cout << "OPTION: PATH TO SOURCEFILE\n"
		        << "\n"
			    << "Syntax: -s --sourcefile [\"filename\"]\n"
				<< "Default value: none\n"
				<< "\n"
				<< "This option provides the path to chosen file with code. "
				<< "Normally the path can be passed as intepreter's first parameter.";
	}
	else if(help_opt == "input" || help_opt == "i" || help_opt == "sourcecode")
	{
	  std::cout << "OPTION: CODE TO EXECUTE\n"
		        << "\n"
			    << "Syntax: -i --input --sourcecode [code]\n"
				<< "Default value: none\n"
				<< "\n"
				<< "This option provides code that will be executed. "
				<< "Normally the code can be passed as intepreter's first parameter, but interpreter have to distinguish between code and filename pattern.";
	}
	else if(help_opt == "nopause")
	{
	  std::cout << "OPTION: NO PAUSE\n"
		        << "\n"
			    << "Syntax: --nopause\n"
				<< "Default value: Flag is not set\n"
				<< "\n"
				<< "This flag turns off a message 'Press any key to continue...' at the very end of program, enforcing "
				<< "completely silent execution. When interpreter is running from console, this message won't be shown, "
				<< "regardless of --nopause flag state.";
	}
	else if(help_opt == "verbose" || help_opt == "silent")
	{
	  std::cout << "OPTION: MESSAGE LEVEL\n"
		        << "\n"
			    << "Syntax: --verbose [all|important|none] | --silent\n"
				<< "Default value: important\n"
				<< "\n"
				<< "This option defines which type of messages will be shown. \"all\": all messages will be shown including flow informations like 'Execution end'. "
				<< "\"important\": only warnings, debugger messages and interpreter's logo will be shown. \"none\": complete silent, transparent execution with "
				<< "no message except program's output. Flag '--silent' is equal to '--message none'";
	}
	else if(help_opt == "log")
	{
	  std::cout << "OPTION: DEBUGGING LOG SETTINGS\n"
		        << "\n"
			    << "Syntax: --log [none|console|\"filename\"]\n"
				<< "Default value: console\n"
				<< "\n"
				<< "This option can turn on and redirect debug messages output to a specified destination: console or logfile."
				<< "Log is used by debugger special instructions, so when --analyse flag is not present, there is no need to specify "
				<< "this setting.";
	}
	//subjects
	else if(help_opt == "exceptions")
	{
	  std::cout << "SUBJECT: RUNTIME EXCEPTIONS\n"
		        << "\n"
			    << "During execution of program\n"
				<< "\n"
				<< "This option can turn on and redirect logging process to a specified destination: console or logfile."
				<< "Log is used by debugger special instructions, so when --debug flag is not present, there is no need to specify "
				<< "this setting.";
	}
	else
	{
	  std::cout << "Type --help [option] to get option's description (p.ex --help eof).\n"
		        //<< "Type --help ['exceptions'] to read about listed subjects.\n"
			    << "\t++ EXECUTION OPTIONS ++\n"
			    << "-l --language [bt|b|bf|pb|brainthread|brainfuck|brainfork|pbrain|auto] Default: brainthread\n"
				<< "-e --eof [0|255|nochange] Default: 0\n"
				<< "-m --memorysize <1, 2^32> Default: 30000\n"
				<< "-b --memorybehavior [constant|tapeloop|dynamic] Default: constant\n"
				<< "-c --cellsize [8|16|32|u8|u16|u32] Default: 8\n"
				<< "--strict      \tDefault: flag is not set\n"
				<< "\n"
				<< "\t++ ENVIROMENT OPTIONS ++\n"
				<< "-a --analyse  \tDefault: flag is not set\n"
				<< "-r --repair   \tDefault: flag is not set\n"
				<< "-x --execute  \tDefault: flag is set\n"
				<< "-s --sourcefile [\"filename\"] Default: not used\n"
				<< "-i --input [code], --sourcecode [code] Default: not used\n"
				<< "--nopause     \tDefault: flag is not set\n"
				<< "--verbose [all|important|none], --silent Default: important\n"
				<< "--log [none|console|\"filename\"] Default: console\n";
	}

	std::cout << std::endl;
}

void ShowInfo(void)
{
	PrintBrainThreadInfoEx();

	std::cout << "\n++ DESCRIPTION OF BRAINTHREAD LANGUAGE ++\n"
			  << "Brainthread is a derivative of Brainfuck, supports functions (from pBrain), threads (like Brainfork) and heaps. "
			  << "All Brainthread programs are also valid Brainfuck programs, although they may (of course) behave very differently.\n"
			  << "Each thread has it's own separate memory and heap. Threads can use and communicate each other by one global shared heap.\n"

			  << "\n++ Basic commands ++\n"
			  << " > increment the data pointer (to point to the next cell to the right).\n"
			  << " < decrement the data pointer (to point to the next cell to the left).\n"
			  << " + increment (increase by one) the cell at the data pointer.\n"
			  << " - decrement (decrease by one) the cell at the data pointer.\n"
			  << " . output the cell at the data pointer.\n"
			  << " , accept one byte of input, storing its value in the cell at the data pointer.\n"
			  << " [ if the cell at the data pointer is zero, then jump it forward to the command after the matching ] command.\n"
			  << " ] if the cell at the data pointer is nonzero, then jump it back to the command after the matching [ command.\n"

			  << "\n++ Threading commands ++\n"
			  << " { the current thread forks and child continues to execute from this point.\n"
			  << " } the current thread waits for all it's children threads.\n"
			  << " ! terminate the current thread. If this is the main thread, terminate the program itself.\n"
			  << "A thread forks with the cell at the data pointer being zeroed and the memory being copied. In the child the data pointer being"
			  << "moved one to the right and that cell set to 1. Heaps aren't copied.\n\n"

			  << "\n++ Function commands ++\n"
			  << " ( begin of function definition\n"
			  << " ) end of function definition\n"
			  << " * call the function of which the identifier is equal to the current cell value.\n"
			  << "A procedure is delimited by the ( and ) instructions. It's assigned a numeric identifier equal to the contents of the active"
			  << "cell at the time the ( is encountered. Threads spawned from the procedure terminates with it's end.\n"

			  << "\n++ Heap commands ++\n"
			  << " & push the current cell value to the heap\n"
			  << " ^ pop the first element of the heap to the current cell. If the heap is empty, 0 is assigned.\n"
			  << " % swap two first elements of the heap. If the heap contains less than 2 values, nothing happens.\n"
			  << " ~ the next heap command refers to shared heap. (from the perspective of the code, not of the flow)\n"
			  << "Use the swap command p.ex to check if the heap is either empty or it's elements are zero\n"

			  << "\n++ Additional commands ++\n"
			  << " : output the cell at the data pointer as integer\n"
			  << " ; accept one integer of input, storing its value in the cell at the data pointer.\n"
			  << "An easy way to read and write numbers."

			  << std::endl;
}