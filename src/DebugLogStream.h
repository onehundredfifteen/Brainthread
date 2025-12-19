#pragma once

#include <iostream>
#include <fstream>
#include <string>

namespace BT {

	class DebugLogStream
	{
	public:
		enum class stream_type
		{
			lsFile,
			lsConsole,
			lsNone
		};

		static DebugLogStream& Instance()
		{
			static DebugLogStream instance;
			return instance;
		}
	private:
		DebugLogStream() : StreamType(stream_type::lsNone) {}
		~DebugLogStream();

		DebugLogStream(DebugLogStream const&) = delete;
		DebugLogStream& operator=(DebugLogStream const&) = delete;

	public:
		void Init(stream_type st, const std::string &lp);
		std::ostream& GetStream();

	private:
		stream_type StreamType;

		std::string log_path;
		std::ofstream stream;

		std::ostream& Stream();
		std::string GetTime();
	};

	template < typename C >
	static void PrintCellValue(std::ostream& o, C ch) {
		bool isprintable = !(ch == 8 || ch == 10 || ch == 13 || ch == 27 || static_cast<unsigned char>(ch) == 255);

		if (sizeof(C) == 1 || (sizeof(C) > 1 && static_cast<unsigned char>(ch) <= 255))
		{
			if constexpr (std::is_signed<C>::value)
				o << static_cast<signed>(ch) << "'" << (isprintable ? static_cast<char>(ch) : ' ') << '\'';
			else
				o << static_cast<unsigned>(ch) << "'" << (isprintable ? static_cast<unsigned char>(ch) : ' ') << '\'';
		}
		else
		{
			if constexpr (std::is_signed<C>::value)
				o << static_cast<int>(ch);
			else
				o << static_cast<unsigned int>(ch);
		}
	}
}