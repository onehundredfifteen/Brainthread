#include "MessageLog.h"

#include <iostream>

namespace BT {
	void MessageLog::SetMessageLevel(MessageLevel message_level)
	{
		this->message_level = message_level;
	}

	void MessageLog::AddMessage(ErrCode e_code, unsigned int pos)
	{
		messages.emplace_back(Error(e_code, pos));
	}

	void MessageLog::AddMessage(ErrCode e_code, const std::string& text)
	{
		messages.emplace_back(Error(e_code, text));
	}

	void MessageLog::AddMessage(const std::string& text)
	{
		AddMessage(MessageLog::ecMessage, text);
	}

	void MessageLog::AddInfo(const std::string& text)
	{
		AddMessage(MessageLog::ecInformation, text);
	}

	unsigned int MessageLog::ErrorsCount(void) const
	{
		unsigned int cnt = 0;
		for (Error msg : messages) {
			if (msg.IsWarning() == false && msg.IsMessage() == false)
				++cnt;
		}
		return cnt;
	}
	unsigned int MessageLog::WarningsCount(void) const
	{
		unsigned int cnt = 0;
		for (Error msg : messages) {
			if (msg.IsWarning())
				++cnt;
		}
		return cnt;
	}
	unsigned int MessageLog::MessagesCount(void) const
	{
		return messages.size();
	}

	void MessageLog::PrintMessages(void) const
	{
		if (message_level == MessageLevel::mlNone)
			return;

		const unsigned int max_msgs = 20;
		unsigned int msgcnt = 0;
		unsigned int errcnt = ErrorsCount();
		unsigned int warcnt = WarningsCount();

		if (errcnt || warcnt)
			std::cout << std::endl << errcnt << ((errcnt > 1) ? " errors, " : " error, ") << warcnt << ((warcnt > 1) ? " warnings." : " warning.") << std::endl;

		for (std::list<Error>::const_iterator it = messages.begin(); it != messages.end(); ++it)
		{
			if (++msgcnt > max_msgs) {
				std::cout << "Too many warning or error messages." << std::endl;
				break;
			}

			if (it->IsMessage() == false)
			{
				std::cout << ((it->IsWarning()) ? "Warning " : "Error ") << (unsigned)it->error_code << ": "
					<< MapMessages(it->error_code);

				if (it->IsGeneralError())
					std::cout << " " << it->text << std::endl;
				else
					std::cout << " at instruction " << it->instruction_pos << std::endl;
			}
			else if (it->error_code == MessageLog::ecMessage)
			{
				std::cout << "Message: " << it->text << "." << std::endl;
			}
			else if (message_level == MessageLevel::mlAll && it->error_code == MessageLog::ecInformation)
			{
				std::cout << "Info: " << it->text << "." << std::endl;
			}
		}
	}

	void MessageLog::ClearMessages(void)
	{
		messages.clear();
	}

	const char* MessageLog::MapMessages(const ErrCode& ec) const
	{
		switch (ec)
		{
		case ecUnmatchedLoopBegin: return "Mismatched loop begin - bracket sign [";
		case ecUnmatchedLoopEnd: return "Mismatched loop end - bracket sign ]";
		case ecUnmatchedFunBegin: return "Mismatched function begin - parenthesis sign (";
		case ecUnmatchedFunEnd: return "Mismatched function end - parenthesis sign )";

		case ecELOutOfFunctionScope: return "Loop end out of function scope - ( [ ) ]";
		case ecBLOutOfFunctionScope: return "Loop begin out of function scope - [ ( ] )";
		case ecUnmatchedBreak: return "Mismatched break";
		case ecUnexpectedSwitch: return "Expected a heap instruction: &,^,%";
		case ecUnexpectedPragma: return "Expected an integer value after #";
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
		case ecJoinBeforeFork: return "Join before fork";

		case ecRedundantArithmetic: return "Redundant arithmetics";
		case ecFunctionInLoop: return "Function declaration in the loop";
		case ecRedundantNearLoopArithmetic: return "Redundant arithmetics before loop like -[+]";
		case ecSlowLoop: return "Loop approaches infinity or very slow loop";

		case ecRedundantMoves: return "Redundant pointer moves like ><><";
		case ecRedundantOpBeforeFork: return "Operation on cell value before fork has no effect";
		case ecInfinityRecurention: return "Funcion calls itself recursively";
		case ecCallButNoFunction: return "Call but no function defined";

		case ecIntegrityLost: return "Code lost integrity. Rerun program with no repair option";

			//ogolne b³edy
		case ecFatalError: return "Fatal Error";
		case ecArgumentError: return "Argument Error";
		case ecUnknownError: return "Unknown Error";
		}
		return "";
	}
}