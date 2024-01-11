#include <windows.h>
#include <process.h>
#include "Settings.h"
#include "BrainThreadExceptions.h"

namespace BT {
	bool Settings::InitFromArguments(GetOpt::GetOpt_pp& ops)
	{	
		try
		{
			std::string op_arg;
			std::vector<std::string> op_args;
			unsigned long long op_arg_i;

			// -e --eof [0|255|nochange]
			if (ops >> GetOpt::OptionPresent('e', "eof"))
			{
				ops >> GetOpt::Option('e', "eof", op_arg);
				if (op_arg == "0")
					OP_eof_behavior = eof_option::eoZero;
				else if (op_arg == "255")
					OP_eof_behavior = eof_option::eoMinusOne;
				else if (op_arg == "nochange")
					OP_eof_behavior = eof_option::eoUnchanged;
				else
					throw BrainThreadInvalidOptionException("eof", op_arg);
			}

			// -c --cellsize [8|16|32|u8|u16|u32]
			if (ops >> GetOpt::OptionPresent('c', "cellsize"))
			{
				ops >> GetOpt::Option('c', "cellsize", op_arg);
				if (op_arg == "8")
					OP_cellsize = cellsize_option::cs8;
				else if (op_arg == "16")
					OP_cellsize = cellsize_option::cs16;
				else if (op_arg == "32")
					OP_cellsize = cellsize_option::cs32;
				else if (op_arg == "u8")
					OP_cellsize = cellsize_option::csu8;
				else if (op_arg == "u16")
					OP_cellsize = cellsize_option::csu16;
				else if (op_arg == "u32")
					OP_cellsize = cellsize_option::csu32;
				else
					throw BrainThreadInvalidOptionException("cellsize", op_arg);
			}

			// -m --memorysize <1,2^32>
			if (ops >> GetOpt::OptionPresent('m', "memorysize"))
			{
				ops >> GetOpt::Option('m', "memorysize", op_arg);
				op_arg_i = atoi(op_arg.c_str());

				if (op_arg_i < 1 || op_arg_i > UINT_MAX)
					throw BrainThreadInvalidOptionException("memorysize", op_arg);
				else
					OP_mem_size = (unsigned int)op_arg_i;
			}

			// -b --memorybehavior [constant|tapeloop|dynamic]
			if (ops >> GetOpt::OptionPresent('b', "memorybehavior"))
			{
				ops >> GetOpt::Option('b', "memorybehavior", op_arg);
				if (op_arg == "constant")
					OP_mem_behavior = mem_option::moLimited;
				else if (op_arg == "tapeloop")
					OP_mem_behavior = mem_option::moContinuousTape;
				else if (op_arg == "dynamic")
					OP_mem_behavior = mem_option::moDynamic;
				else
					throw BrainThreadInvalidOptionException("memorybehavior", op_arg);
			}

			// -l --language [bt|b|bf|pb|brainthread|brainfuck|brainfork|pbrain|auto]
			if (ops >> GetOpt::OptionPresent('l', "language"))
			{
				ops >> GetOpt::Option('l', "language", op_arg);
				if (op_arg == "bt" || op_arg == "brainthread")
					OP_language = CodeLang::clBrainThread;
				else if (op_arg == "b" || op_arg == "brainfuck")
					OP_language = CodeLang::clBrainFuck;
				else if (op_arg == "bf" || op_arg == "brainfork")
					OP_language = CodeLang::clBrainFork;
				else if (op_arg == "pb" || op_arg == "pbrain")
					OP_language = CodeLang::clPBrain;
				else
					throw BrainThreadInvalidOptionException("language", op_arg);
			}
			//opcje œrodowiska

			//debug, repair & execute
			OP_analyse = (ops >> GetOpt::OptionPresent('a', "analyse"));

			OP_optimize = (ops >> GetOpt::OptionPresent('o', "optimize")); //todo o2 o3
			OP_repair = OP_optimize || (ops >> GetOpt::OptionPresent('r', "repair")); //niekoniecznie chce, aby debug naprawia³
			OP_execute = (ops >> GetOpt::OptionPresent('x', "execute"));  //niekoniecznie chce, aby po debugu uruchamia³

			if (OP_optimize || OP_repair)//aby by³ repair, musi byc debug
				OP_analyse = true;
			if (OP_analyse == false)//nie debugujesz? musi byc execute
				OP_execute = true;

			//--verbose[all|important|none]
			//--wall
			//--silent
			if (ops >> GetOpt::OptionPresent("verbose") || ops >> GetOpt::OptionPresent("silent"))
			{
				if (ops >> GetOpt::OptionPresent("verbose"))
				{
					ops >> GetOpt::Option("verbose", op_arg);
					if (op_arg == "all")
						OP_message = MessageLog::MessageLevel::mlAll;
					else if (op_arg == "important")
						OP_message = MessageLog::MessageLevel::mlImportant;
					else if (op_arg == "none")
						OP_message = MessageLog::MessageLevel::mlNone;
					else
						throw BrainThreadInvalidOptionException("verbose", op_arg);
				}

				if (ops >> GetOpt::OptionPresent("silent"))
				{
					OP_message = MessageLog::MessageLevel::mlNone;
				}
			}

			MessageLog::Instance().SetMessageLevel(OP_message);

			// --log [none|console|filename]
			if (ops >> GetOpt::OptionPresent("log"))
			{
				ops >> GetOpt::Option("log", op_arg);

				if (op_arg.find(".") != std::string::npos) //rozpoznajemy ¿e wpisano plik
					OP_log = DebugLogStream::lsFile;
				else if (op_arg == "none")
					OP_log = DebugLogStream::lsNone;
				else if (op_arg == "console")
					OP_log = DebugLogStream::lsConsole;
				else
					throw BrainThreadInvalidOptionException("log", op_arg);

				DebugLogStream::Instance().Init(OP_log, op_arg);
			}
			else
			{
				DebugLogStream::Instance().Init(OP_log, "");
			}

			//-s --sourcefile [filename]
			//-i --input [code]
			//--sourcecode [code]
			if (ops >> GetOpt::OptionPresent('s', "sourcefile"))
			{
				ops >> GetOpt::Option('s', "sourcefile", op_arg);
				if (!GetCodeFromFile(op_arg)) {
					throw BrainThreadOptionNotEligibleException("sourcefile", op_arg, "File does not exist");
				}
			}
			else if (ops >> GetOpt::OptionPresent('i', "input") || ops >> GetOpt::OptionPresent("sourcecode")) //-i --input --sourcefile [code]
			{
				if (ops >> GetOpt::OptionPresent('i', "input"))
					ops >> GetOpt::Option('i', "input", op_arg);
				else
					ops >> GetOpt::Option("sourcecode", op_arg);

				OP_source_code = op_arg;
			}

			//--strict //super zgodne ustawienie opcji
			if (ops >> GetOpt::OptionPresent("strict"))
			{
				switch (OP_language) {
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
			if (ops >> GetOpt::OptionPresent("nopause") == false)
			{
				OP_nopause = IsRanFromConsole(); //nie ma opcji - pozwól samemu wykminic czy jest potrzeba
			}

			//reszta opcji, w³aœciwie spodziewany sie jednej tylko
			if (ops.options_remain())
			{
				ops >> GetOpt::GlobalOption(op_args);

				if (op_args.size() > 1)
					throw GetOpt::TooManyArgumentsEx();
				else if (!GetCodeFromFile(op_args[0])) {
					throw GetOpt::InvalidFormatEx();
				}
			}
			return true;
		}
		catch (GetOpt::TooManyArgumentsEx &ex)
		{
			MessageLog::Instance().AddMessage(MessageLog::ecArgumentError, "Too many arguments");
			return false;
		}
		catch (GetOpt::GetOptEx &ex)
		{
			MessageLog::Instance().AddMessage(MessageLog::ecArgumentError, "Error while parsing arguments: " + std::string(ex.what()));
			return false;
		}
		catch (BrainThreadInvalidOptionException &ex)
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

	bool Settings::GetCodeFromFile(const std::string& filepath)
	{
		std::ifstream in(filepath);
		if (in.fail())
			return false;

		OP_source_file_path = filepath;
		OP_source_code = "";

		std::copy(std::istream_iterator<char>(in), std::istream_iterator<char>(), std::back_inserter(OP_source_code));
		in.close();

		return true;
	}

	bool Settings::IsRanFromConsole()
	{
		HWND consoleWnd = GetConsoleWindow();
		DWORD dwProcessId;
		GetWindowThreadProcessId(consoleWnd, &dwProcessId);

		return (!(GetCurrentProcessId() == dwProcessId));
	}
}