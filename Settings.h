#pragma once

#include <string>
#include "Enumdefs.h"
#include "DebugLogStream.h"
#include "MessageLog.h"

#pragma warning(push, 0)
#include "getoptpp/getopt_pp_standalone.h"
#pragma warning(pop)

namespace BT {

	struct Settings
	{
		bool OP_analyse = false;
		bool OP_repair = false;
		bool OP_optimize = false;
		bool OP_execute = true;
		bool OP_nopause = Settings::IsRanFromConsole();

		MessageLog::MessageLevel OP_message = MessageLog::MessageLevel::mlImportant;
		DebugLogStream::stream_type OP_log = DebugLogStream::stream_type::lsConsole;

		std::string OP_source_code = "";
		std::string OP_source_file_path = "";

		CodeLang OP_language = CodeLang::clBrainThread;

		mem_option OP_mem_behavior = mem_option::moLimited;
		eof_option OP_eof_behavior = eof_option::eoZero;
		cellsize_option OP_cellsize = cellsize_option::cs8;

		unsigned int OP_mem_size = def_mem_size;
		

		bool InitFromArguments(GetOpt::GetOpt_pp& ops);
		bool GetCodeFromFile(const std::string &filepath);

		static bool IsRanFromConsole();
		static const int def_mem_size = 30000;
	};
}
