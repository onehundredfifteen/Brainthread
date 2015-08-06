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
			lsConsole,
			lsNone
		} stream_type;

	private: //singleton
		LogStream();
		LogStream(const LogStream &);
		LogStream& operator=(const LogStream&);
	    ~LogStream(void);

	public:
		static LogStream& GetInstance();
		void OpenStream(stream_type st, std::string lp);

		std::ostream& GetStream();

	private:
		stream_type StreamType;

		std::string LogPath;
		std::ofstream stream;

		char  time_buf[40];

		std::ostream& Stream();
		std::string GetTime();
};