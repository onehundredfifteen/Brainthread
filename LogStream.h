#pragma once

#include <iostream>
#include <fstream>
#include <string>

class LogStream
{
	public:
		typedef enum
		{
			lsFile,
			lsConsole
		} stream_type;

		LogStream(stream_type st) : StreamType(st){}
		~LogStream();

		void SetLogPath(std::string lp);

		std::ostream& GetStream();
		std::ostream& GetStreamSession();

	private:
		const stream_type StreamType;
		std::string LogPath;
		std::ofstream stream;

		char  time_buf[40];

		std::string GetTime();
};