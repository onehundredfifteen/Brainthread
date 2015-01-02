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
unsigned ParseErrors::MessagesCount(void)
{
	return messages.size();
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
		case ecUnmatchedFunBegin: return "Mismatched function begin - parenthesis sign (";
        case ecUnmatchedFunEnd: return "Mismatched function end - parenthesis sign )";

		case ecInfiniteLoop: return "Detected an infinite loop like []";
        case ecEmptyLoop: return "Detected an empty loop like [[xx]]";
		case ecELOutOfFunctionScope: return "Loop end out of function scope - ( [ ) ]";
        case ecBLOutOfFunctionScope: return "Loop begin out of function scope - [ ( ] )";
			
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