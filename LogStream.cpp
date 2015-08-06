#include "LogStream.h"

#include <ctime>

LogStream::LogStream()
{
	StreamType = lsNone;
}
LogStream::~LogStream()
{
	if(StreamType == LogStream::lsFile && stream.is_open())
		stream.close();
}

LogStream& LogStream::GetInstance()
{
	static LogStream instance;
    return instance;
}

void LogStream::OpenStream(stream_type st, std::string lp)
{
	StreamType = st;

	if(StreamType == LogStream::lsFile)
		LogPath = lp;
}

std::ostream& LogStream::Stream() 
{
	if(StreamType == LogStream::lsFile)
	{
		if(stream.is_open() == false)
		{
			stream.open(LogPath, std::ios::app);	
			if(stream.good() == false)
			{
				std::cout << "\n>Thread ID: unknown. LogStream error\n";
				return std::cout;
			}
		}
		return stream;
	}
	else return std::cout;
}

std::ostream& LogStream::GetStream() 
{
	if(StreamType == LogStream::lsFile)
	{
		Stream() << "\n++ Debug Log " << GetTime() << " ++\n";
	}
	return Stream();
}

std::string LogStream::GetTime()
{
	time_t t = time(0);   // get time now
    struct tm * now = localtime( &t );

	strftime(time_buf, sizeof(time_buf), "%Y-%m-%d.%X", now);
	
	return time_buf;
}