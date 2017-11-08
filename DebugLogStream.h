#pragma once

#include <iostream>
#include <fstream>
#include <string>

class DebugLogStream
{
	public:
		typedef enum
		{
			lsFile,
			lsConsole,
			lsNone
		} stream_type;
    
		static DebugLogStream & Instance()
		{
			static DebugLogStream instance;

			return instance;
		}
	private: 
		DebugLogStream(){
			StreamType = lsNone;
		}

		DebugLogStream(DebugLogStream const&);
		DebugLogStream& operator=(DebugLogStream const&);
	    ~DebugLogStream(void);

	public:
		void Init(stream_type st, std::string lp);
		std::ostream& GetStream();

	private:
		stream_type StreamType;

		std::string LogPath;
		std::ofstream stream;

		std::ostream& Stream();
		std::string GetTime();
};