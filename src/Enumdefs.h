#pragma once

namespace BT {
	enum class cellsize_option
	{
		cs8, cs16, cs32, csu8, csu16, csu32
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
}