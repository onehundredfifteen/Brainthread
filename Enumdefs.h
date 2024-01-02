#pragma once

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
}