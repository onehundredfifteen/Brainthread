#pragma once

#include <list>
#include <string>

namespace BT {
	class MessageLog
	{
	public:
		enum class ErrCode
		{
			ecUnmatchedLoopBegin = 100,
			ecUnmatchedLoopEnd,
			ecUnmatchedFunBegin,
			ecUnmatchedFunEnd,
			ecBLOutOfFunctionScope,
			ecELOutOfFunctionScope,
			ecUnmatchedBreak,
			ecUnexpectedSwitch,
			ecUnexpectedPragma,

			ecEmptyCode,
			ecIntegrityLost = 190,
			ecFatalError, 
			ecArgumentError,
			ecUnknownError,

			//warningi
			ecInfiniteLoop = 200,
			ecEmptyLoop,
			ecEmptyFunction,
			ecFunctionRedefinition,
			ecFunctionRedefinitionInternal,

			ecFunctionInLoop,
			ecFunctionLimitExceed,
			ecFunctionExistsButNoCall,
			ecInfinityRecurention,
			ecJoinButNoFork,

			ecTerminateRepeat,
			ecJoinRepeat,
			ecSwapRepeat,
			ecJoinBeforeFork,

			ecRedundantArithmetic,
			ecRedundantNearLoopArithmetic,
			ecSlowLoop,
			ecRedundantMoves,
			ecRedundantOpBeforeFork,

			//ecRedundantSwitch,
			//ecSwitchRepeat,
			//ecSwithOutOfScope,
			ecCallButNoFunction,
			ecPragmaValueTooBig,
			ecPragmaValueTooSmall,
			ecPragmaUnsupported,

			ecMessage = 600,
			ecInformation = 700
		};

		enum class MessageLevel
		{
			mlAll,
			mlImportant,
			mlNone
		};

		struct Message
		{
			const unsigned int instruction_pos;
			const ErrCode error_code;
			const std::string text;

			Message(ErrCode ec, const std::string& msg, unsigned int pos)
				: error_code(ec), instruction_pos(pos), text(msg) {}
			Message(ErrCode ec, const std::string& msg)
				: Message(ec, msg, 0) {}
			Message(ErrCode ec, unsigned int pos)
				: Message(ec, "", pos) {}
			Message(const std::string& msg)
				: Message(ErrCode::ecMessage, msg) {}

			bool IsWarning(void) const {
				return (int)error_code >= 200 && (int)error_code < (int)ErrCode::ecMessage;
			}
			bool IsMessage(void) const {
				return error_code == ErrCode::ecMessage || 
					error_code == ErrCode::ecInformation;
			}
			bool IsError(void) const {
				return (int)error_code < 200;
			}
			bool IsGeneralError(void) const {
				return error_code == ErrCode::ecIntegrityLost || 
					error_code == ErrCode::ecFatalError ||
					error_code == ErrCode::ecUnknownError || 
					error_code == ErrCode::ecArgumentError;
			}
		};

	public:
		static MessageLog& Instance()
		{
			static MessageLog instance;

			return instance;
		}

		void SetMessageLevel(MessageLevel message_level);

		void AddMessage(ErrCode e_code, unsigned int pos);
		void AddMessage(ErrCode e_code, const std::string& text);
		void AddMessage(const std::string& text);
		void AddInfo(const std::string& text);

		unsigned int ErrorsCount(void) const;
		unsigned int WarningsCount(void) const;
		unsigned int MessagesCount(void) const;
		void PrintMessages(void) const;
		void ClearMessages(void);

	private:
		MessageLog() {
			message_level = MessageLevel::mlImportant;
		}

		MessageLog(MessageLog const&) = delete;
		MessageLog& operator=(MessageLog const&) = delete;

	protected:
		std::list<Message> messages;
		MessageLevel message_level;

		const char* MapMessages(const ErrCode& ec) const;
	};
}