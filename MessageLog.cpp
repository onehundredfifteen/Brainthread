#include "MessageLog.h"

#include <iostream>

MessageLog::MessageLog(bool iom)
{
	error_count = 0;
	warning_count = 0;

	important_messages_only = iom;
}

MessageLog::~MessageLog(void)
{
}

void MessageLog::AddMessage(ErrCode e_code, unsigned int pos, unsigned int line)
{
	Error e(e_code, pos / line, line);
	
	messages.push_back(e);

	if(e.IsWarning() == false && e.IsMessage() == false)
		++error_count;
	else if(e.IsWarning() == true)
		++warning_count;
}

void MessageLog::AddMessage(ErrCode e_code, std::string t)
{
	Error e(e_code, t);
	
	messages.push_back(e);
}

unsigned MessageLog::ErrorsCount(void)
{
	return error_count;
}
unsigned MessageLog::WarningsCount(void)
{
	return warning_count;
}
unsigned MessageLog::MessagesCount(void)
{
	return messages.size();
}

void MessageLog::GetMessages(void)
{
	std::cerr << ErrorsCount() << ((ErrorsCount() != 1)? " errors, " : " error, ") << WarningsCount() << ((WarningsCount() != 1)? " warnings." : " warning.") << std::endl;
	unsigned mcnt = 0;

	for(std::vector<Error>::iterator it = messages.begin(); it != messages.end(); ++it)
	{
		if(it->IsMessage() == false)
		{
		    if(++mcnt > 20)
			{
				std::cerr << "Too many warning or error messages." << std::endl;
				break;
			}
		
			std::cerr << ((it->IsWarning())? "Warning " : "Error ") << (unsigned)it->error_code << ": " 
						  << MapMessages(it->error_code);

			if(it->line > 0)
				std::cerr <<". Line: " << it->line << " Col: " << it->col << std::endl;
			else 
				std::cerr <<". Command: " << it->col << std::endl;
		}
		else //if(important_messages_only == true || it->error_code == ecMessage)
		{
			std::cerr << "Message " << (unsigned)it->error_code << ": " << it->text << "." << std::endl;
		}
	}
}

const char*  MessageLog::MapMessages(ErrCode &ec) const
{
	switch(ec)
	{
		case ecLoopUnmatchedR: return "Mismatched loop begin - bracket sign [";
        case ecLoopUnmatchedL: return "Mismatched loop end - bracket sign ]";
		case ecUnmatchedFunBegin: return "Mismatched function begin - parenthesis sign (";
        case ecUnmatchedFunEnd: return "Mismatched function end - parenthesis sign )";
        
		case ecELOutOfFunctionScope: return "Loop end out of function scope - ( [ ) ]";
        case ecBLOutOfFunctionScope: return "Loop begin out of function scope - [ ( ] )";
		case ecInfiniteLoop: return "Detected an infinite loop like []";
        case ecEmptyLoop: return "Detected an empty loop like [[xx]]";
		
			
		case ecEmptyFunction: return "Detected an empty function loop like ()";
        case ecFunctionRedefinition: return "Suspected function redeclaration";
		case ecFunctionRedefinition2: return "Suspected internal function redeclaration";
		case ecFunctionLimitExceed: return "Function name pool exceed";
        case ecFunctionExistsButNoCall: return "A function was declared but no call in code";

		case ecJoinButNoFork: return "A join command exists but no fork";
		case ecTerminateRepeat: return "Unessesary terminate repeat like !!";
		case ecJoinRepeat: return "Unessesary join repeat like }}";
        case ecJoinBeforeFork: return "Join before fork";
			
		case ecRedundantArithmetic: return "Redundant arithmetics";
		case ecRedundantLoopArithmetic: return "Redundant arithmetics in loop - possibly infinite";
		case ecRedundantNearLoopArithmetic: return "Redundant arithmetics near loop like -[-]";
        case ecSlowLoop: return "Very slow loop like [+]";	

		case ecRedundantMoves: return "Redundat pointer moves like ><><";	
		
	}
	return "";
}