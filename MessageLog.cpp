#include "MessageLog.h"

#include <iostream>

void MessageLog::SetMessageLevel(MessageLevel message_level)
{
	this->message_level = message_level;
}

void MessageLog::AddMessage(ErrCode e_code, unsigned int pos)
{
	Error e(e_code, pos);
	
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

void MessageLog::AddMessage(std::string t)
{
	AddMessage(MessageLog::ecMessage, t);
}

void MessageLog::AddInfo(std::string t)
{
	AddMessage(MessageLog::ecInformation, t);
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
	unsigned mcnt = 0;
	
	if(message_level == MessageLog::mlNone)
		return;//¿adnych wiadomoœci
	
	if(ErrorsCount() || WarningsCount() )
		std::cout << std::endl << ErrorsCount() << ((ErrorsCount() != 1)? " errors, " : " error, ") << WarningsCount() << ((WarningsCount() != 1)? " warnings." : " warning.") << std::endl;

	for(std::vector<Error>::iterator it = messages.begin(); it != messages.end(); ++it)
	{
		if(it->IsMessage() == false)
		{
		    if(++mcnt > 20)
			{
				std::cout << "Too many warning or error messages." << std::endl;
				break;
			}
		
			std::cout << ((it->IsWarning())? "Warning " : "Error ") << (unsigned)it->error_code << ": " 
						  << MapMessages(it->error_code);

			if(it->IsGeneralError())
				std::cout << " " << it->text << std::endl;
			else
				std::cout <<" at instruction " << it->instruction_pos << std::endl;
		}
		else if(it->error_code == MessageLog::ecMessage)
		{
			std::cout << "Message: " << it->text << "." << std::endl;
		}
		else if(message_level == MessageLog::mlAll && it->error_code == MessageLog::ecInformation)
		{
			std::cout << "Info: " << it->text << "." << std::endl;
		}
	}
}

const char*  MessageLog::MapMessages(ErrCode &ec) const
{
	switch(ec)
	{
		case ecUnmatchedLoopBegin: return "Mismatched loop begin - bracket sign [";
        case ecUnmatchedLoopEnd: return "Mismatched loop end - bracket sign ]";
		case ecUnmatchedFunBegin: return "Mismatched function begin - parenthesis sign (";
        case ecUnmatchedFunEnd: return "Mismatched function end - parenthesis sign )";
        
		case ecELOutOfFunctionScope: return "Loop end out of function scope - ( [ ) ]";
        case ecBLOutOfFunctionScope: return "Loop begin out of function scope - [ ( ] )";
        case ecUnmatchedBreak: return "Mismatched break";
		case ecEmptyCode: return "Source code is empty";

		case ecInfiniteLoop: return "Detected an infinite loop like []";
        case ecEmptyLoop: return "Detected an empty loop like [[xx]]";
		
		case ecEmptyFunction: return "Detected an empty function like ()";
        case ecFunctionRedefinition: return "Suspected function redeclaration";
		case ecFunctionRedefinitionInternal: return "Suspected internal function redeclaration";
		case ecFunctionLimitExceed: return "The set of names for the functions can be exceeded";
        case ecFunctionExistsButNoCall: return "A function was declared but not used";

		case ecJoinButNoFork: return "A join command exists but there is no fork";
		case ecTerminateRepeat: return "Unnecessary terminate instruction repetition";
		case ecJoinRepeat: return "Unnecessary join instruction repetition";
		case ecSwapRepeat: return "Unnecessary swap instruction repetition";
		case ecSwitchRepeat: return "Unnecessary switch to shared heap instruction repetition";
        case ecJoinBeforeFork: return "Join before fork";
			
		case ecRedundantArithmetic: return "Redundant arithmetics";
		case ecFunctionInLoop: return "Function declaration in the loop";
		case ecRedundantNearLoopArithmetic: return "Redundant arithmetics before loop like -[+]";
        case ecSlowLoop: return "Loop approaches infinity or very slow loop";	

		case ecRedundantMoves: return "Redundant pointer moves like ><><";	
		case ecRedundantOpBeforeFork: return "Operation on cell value before fork has no effect";
		case ecInfinityRecurention: return "Funcion calls itself recursively";
		case ecRedundantSwitch: return "Switch to shared heap instruction has no effect";
		case ecSwithOutOfScope: return "Switch to shared heap instruction is out of scope";

		case ecCallButNoFunction: return "Call but no function defined";

		case ecIntegrityLost: return "Code lost integrity. Rerun program with no repair option";

		//ogolne b³edy
		case ecFatalError: return "Fatal Error";
		case ecArgumentError: return "Argument Error";
		case ecUnknownError: return "Unknown Error";
	}
	return "";
}