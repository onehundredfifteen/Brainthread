#pragma once

#include <string>
#include "DebugLogStream.h"
#include "MessageLog.h"

namespace BT {
	enum class cellsize_option
	{
		cs8, cs16, cs32, csu8, csu16, csu32
	};

	enum class sourcetype_option
	{
		stFile, stInput, stUrl
	};

	enum class mem_option
	{
		moLimited,
		moDynamic,
		moContinuousTape
	};

	enum class eof_option
	{
		eoZero,
		eoMinusOne,
		eoUnchanged
	};

	enum class CodeLang
	{
		clBrainThread,
		clBrainFuck,
		clPBrain,
		clBrainFork,
		clBrainLove
	};

	struct BTFlags {
		bool OP_debug, OP_repair, OP_optimize, OP_execute, OP_nopause;

		MessageLog::MessageLevel OP_message;
		DebugLogStream::stream_type OP_log;
		std::string OP_source = "";
		CodeLang OP_language;

		mem_option OP_mem_behavior;
		eof_option OP_eof_behavior;
		unsigned int OP_mem_size;
		cellsize_option OP_cellsize;
		sourcetype_option OP_sourcetype;
	};
}