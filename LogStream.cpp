#include "LogStream.h"

#include <ctime>

LogStream::~LogStream()
{
	if(StreamType == LogStream::lsFile && stream.is_open())
		stream.close();
}

void LogStream::SetLogPath(std::string lp)
{
	this->LogPath = lp;
}

std::ostream& LogStream::GetStream() 
{
	if(StreamType == LogStream::lsFile)
	{
		if(stream.is_open() == false)
		{
			stream.open(LogPath, std::ios::app);	
			if(stream.good() == false)
			{
				std::cout << "\n>Thread ID: unkown. LogStream error\n";
				return std::cout;
			}
		}
		return stream;
	}
	else return std::cout;
}

std::ostream& LogStream::GetStreamSession() 
{
	if(StreamType == LogStream::lsFile)
	{
		GetStream() << "\n++ Debug Log " << GetTime() << " ++\n";
	}
	return GetStream();
}

std::string LogStream::GetTime()
{
	time_t t = time(0);   // get time now
    struct tm * now = localtime( &t );

	strftime(time_buf, sizeof(time_buf), "%Y-%m-%d.%X", now);
	
	return time_buf;
}