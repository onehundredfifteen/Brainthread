#include "MessageLog.h"

#include <iostream>

namespace BT {
	void MessageLog::SetMessageLevel(MessageLevel message_level)
	{
		this->message_level = message_level;
	}

	void MessageLog::AddMessage(ErrCode e_code, unsigned int pos)
	{
		messages.emplace_back(Message(e_code, pos));
	}

	void MessageLog::AddMessage(ErrCode e_code, const std::string& text)
	{
		messages.emplace_back(Message(e_code, text));
	}

	void MessageLog::AddMessage(const std::string& text)
	{
		messages.emplace_back(Message(text));
	}

	void MessageLog::AddInfo(const std::string& text)
	{
		AddMessage(MessageLog::ErrCode::ecInformation, text);
	}

	unsigned int MessageLog::ErrorsCount(void) const
	{
		unsigned int cnt = 0;
		for (const Message &msg : messages) {
			if (msg.IsError())
				++cnt;
		}
		return cnt;
	}
	unsigned int MessageLog::WarningsCount(void) const
	{
		unsigned int cnt = 0;
		for (const Message& msg : messages) {
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

		if (MessagesCount())
			std::cout << '\n';

		if (errcnt || warcnt)
			std::cout << errcnt << ((errcnt == 1) ? " error, " : " errors, ") << warcnt << ((warcnt == 1) ? " warning." : " warnings.") << std::endl;

		for (const Message& msg : messages)
		{
			if (++msgcnt > max_msgs) {
				std::cout << "Too many warning or error messages." << std::endl;
				break;
			}

			if (msg.IsMessage() == false) {
				std::cout << ((msg.IsWarning()) ? "Warning " : "Error ") << (unsigned)msg.error_code << ": "
					<< MapMessages(msg.error_code);

				if (msg.IsGeneralError())
					std::cout << " " << msg.text << std::endl;
				else
					std::cout << " at instruction " << msg.instruction_pos << std::endl;
			}
			else if (msg.error_code == MessageLog::ErrCode::ecMessage) {
				std::cout << "Message: " << msg.text << "." << std::endl;
			}
			else if (message_level == MessageLevel::mlAll &&
				msg.error_code == MessageLog::ErrCode::ecInformation) {
				std::cout << "Info: " << msg.text << "." << std::endl;
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
		case ErrCode::ecUnmatchedLoopBegin: return "Mismatched loop begin - bracket sign [";
		case ErrCode::ecUnmatchedLoopEnd: return "Mismatched loop end - bracket sign ]";
		case ErrCode::ecUnmatchedFunBegin: return "Mismatched function begin - parenthesis sign (";
		case ErrCode::ecUnmatchedFunEnd: return "Mismatched function end - parenthesis sign )";

		case ErrCode::ecELOutOfFunctionScope: return "Loop end out of function scope - ( [ ) ]";
		case ErrCode::ecBLOutOfFunctionScope: return "Loop begin out of function scope - [ ( ] )";
		case ErrCode::ecUnmatchedBreak: return "Mismatched break";
		case ErrCode::ecUnexpectedSwitch: return "Expected a heap instruction (&, ^, %)";
		case ErrCode::ecUnexpectedPragma: return "Expected an integer value followed by command";
		
		case ErrCode::ecInfiniteLoop: return "Detected an infinite loop like []";
		case ErrCode::ecEmptyLoop: return "Detected an empty loop like [[xx]]";

		case ErrCode::ecEmptyFunction: return "Detected an empty function like ()";
		case ErrCode::ecFunctionRedefinition: return "Suspected function redeclaration";
		case ErrCode::ecFunctionRedefinitionInternal: return "Suspected internal function redeclaration";
		case ErrCode::ecFunctionLimitExceed: return "The set of names for the functions can be exceeded";
		case ErrCode::ecFunctionExistsButNoCall: return "A function was declared but not used";

		case ErrCode::ecJoinButNoFork: return "A join command exists but there is no fork";
		case ErrCode::ecTerminateRepeat: return "Unnecessary terminate instruction repetition";
		case ErrCode::ecJoinRepeat: return "Unnecessary join instruction repetition";
		case ErrCode::ecSwapRepeat: return "Unnecessary swap instruction repetition";
		case ErrCode::ecJoinBeforeFork: return "Join before fork";

		case ErrCode::ecRedundantArithmetic: return "Redundant arithmetics";
		case ErrCode::ecFunctionInLoop: return "Function declaration in the loop";
		case ErrCode::ecRedundantNearLoopArithmetic: return "Redundant arithmetics before loop like -[+]";
		case ErrCode::ecSlowLoop: return "Loop approaches infinity or very slow loop";

		case ErrCode::ecRedundantMoves: return "Redundant pointer moves like ><><";
		case ErrCode::ecRedundantOpBeforeFork: return "Operation on cell value before fork has no effect";
		case ErrCode::ecInfinityRecurention: return "Funcion calls itself recursively";
		case ErrCode::ecCallButNoFunction: return "Call but no function defined";
		case ErrCode::ecPragmaValueTooSmall: return "Pragma value less than 1 have no effect";
		case ErrCode::ecPragmaValueTooBig: return "Pragma value is too big, truncating to 256";
		case ErrCode::ecPragmaUnsupported: return "This instruction is unsupported by pragmas";

		//other errors
		case ErrCode::ecEmptyCode: return "Source code is empty";
		case ErrCode::ecIntegrityLost: return "Code lost integrity. Rerun program with no repair option";
		case ErrCode::ecFatalError: return "Fatal Error";
		case ErrCode::ecArgumentError: return "Argument Error";
		case ErrCode::ecUnknownError: return "Unknown Error";
		}
		return "";
	}
}