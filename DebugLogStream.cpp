#include "DebugLogStream.h"

#include <ctime>

DebugLogStream::~DebugLogStream()
{
	if(StreamType == DebugLogStream::lsFile && stream.is_open())
		stream.close();
}

void DebugLogStream::Init(stream_type st, std::string lp)
{
	StreamType = st;

	if(StreamType == DebugLogStream::lsFile)
		LogPath = lp;
}

std::ostream& DebugLogStream::Stream() 
{
	if(StreamType == DebugLogStream::lsFile)
	{
		if(stream.is_open() == false)
		{
			stream.open(LogPath, std::ios::app);	
			if(stream.good() == false)
			{
				std::cerr << "<main> Debugging log error: cannot open file " << LogPath << std::endl;
				return std::cout;
			}
		}
		return stream;
	}
	else return std::cout;
}

std::ostream& DebugLogStream::GetStream() 
{
	if(StreamType == DebugLogStream::lsFile)
	{
		Stream() << "\n++ Debug Log " << GetTime() << " ++\n";
	}
	return Stream();
}

std::string DebugLogStream::GetTime()
{
	char time_buf[40];
	struct tm* now;
	time_t t = time(0);   // get time now
	
	localtime_s(now, &t);
	strftime(time_buf, sizeof(time_buf), "%Y-%m-%d.%X", now);
	
	return time_buf;
}