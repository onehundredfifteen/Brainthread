#include "ParseErrors.h"

#include <iostream>

ParseErrors::ParseErrors(void)
{
	error_count = 0;
}

ParseErrors::~ParseErrors(void)
{
}

void ParseErrors::AddMessage(ErrCode e_code, unsigned int pos, unsigned int line)
{
	line = line == 0 ? 1 : line; 
	Error e(e_code, pos / line, line);
	
	messages.push_back(e);

	if(e.IsWarning() == false)
		++error_count;
}

unsigned ParseErrors::ErrorsCount(void)
{
	return error_count;
}
unsigned ParseErrors::WarningsCount(void)
{
	return messages.size() - error_count;
}

void ParseErrors::GetMessages(void)
{
	std::cerr << ErrorsCount() << ((ErrorsCount() > 1)? " errors, " : " error, ") << WarningsCount() << ((WarningsCount() > 1)? " warnings." : " warning.") << std::endl;
	unsigned mcnt = 0;

	for(std::vector<Error>::iterator it = messages.begin(); it != messages.end(); ++it)
	{
		if(++mcnt > 20)
		{
			std::cerr << "Too many warning or error messages." << std::endl;
			break;
		}
		
		std::cerr << ((it->IsWarning())? "Warning " : "Error ") << (unsigned)it->error_code << ": " 
				  << MapMessages(it->error_code) 
				  <<". Line: " << it->line << " Col: " << it->col << std::endl;
	}
}

const char*  ParseErrors::MapMessages(ErrCode &ec) const
{
	switch(ec)
	{
		case ecLoopUnmatchedR: return "Mismatched loop begin - bracket sign [";
        case ecLoopUnmatchedL: return "Mismatched loop end - bracket sign ]";
		case ecUnmatchedFunBegin: return "Mismatched begin function - parenthesis sign (";
        case ecUnmatchedFunEnd: return "Mismatched end function - parenthesis sign )";
	}
	return "";
}