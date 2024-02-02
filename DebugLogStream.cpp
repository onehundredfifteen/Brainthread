#include "DebugLogStream.h"

#include <ctime>
#include <chrono>

namespace BT {
	DebugLogStream::~DebugLogStream()
	{
		if (StreamType == stream_type::lsFile && stream.is_open())
			stream.close();
	}

	void DebugLogStream::Init(stream_type st, const std::string &lp)
	{
		StreamType = st;

		if (StreamType == stream_type::lsFile)
			log_path = lp;
	}

	std::ostream& DebugLogStream::GetStream()
	{
		if (StreamType == stream_type::lsFile)
		{
			if (stream.is_open() == false)
			{
				stream.open(log_path, std::ios::app);
				if (stream.good() == false)
				{
					std::cerr << "\n> Cannot log to file " << log_path << std::endl;
					return std::cout;
				}
			}
			stream << "\n> log " << GetTime() << ">\t";
			return stream;
		}
		else return std::cout;
	}

	std::string DebugLogStream::GetTime()
	{
		char time_buf[40];
		struct tm now;
		time_t t = time(0);   // get time now
		localtime_s(&now, &t);
		strftime(time_buf, sizeof(time_buf), "%Y-%m-%d.%X", &now);

		return time_buf;
	}
}