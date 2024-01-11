#pragma once

namespace BT {
	enum class cellsize_option : uint8_t
	{
		cs8, cs16, cs32, csu8, csu16, csu32
	};

	enum class mem_option : uint8_t
	{
		moLimited,
		moDynamic,
		moContinuousTape
	};

	enum class eof_option : uint8_t
	{
		eoZero,
		eoMinusOne,
		eoUnchanged
	};

	enum class CodeLang : uint8_t
	{
		clBrainThread,
		clBrainFuck,
		clPBrain,
		clBrainFork,
		clBrainLove
	};
}