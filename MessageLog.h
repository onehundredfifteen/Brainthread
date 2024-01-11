#pragma once

#include <list>
#include <string>

namespace BT {
	class MessageLog
	{
	public:
		enum ErrCode
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
			ecIntegrityLost,
			ecFatalError = 190,
			ecArgumentError,
			ecUnknownError = 199,

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

			ecMessage = 600,
			ecInformation //taka wiadomoœæ bêdzie pomijana
		};

		enum class MessageLevel
		{
			mlAll,
			mlImportant,
			mlNone
		};

		struct Error
		{
			const unsigned int instruction_pos;
			const ErrCode error_code;
			const std::string text;

			Error(ErrCode ec, const std::string& t, unsigned int pos)
				: error_code(ec), instruction_pos(pos), text(t) {}
			Error(ErrCode ec, const std::string& t)
				: Error(ec, t, 0) {}
			Error(ErrCode ec, unsigned int pos)
				: Error(ec, "", 0) {}
			Error(ErrCode ec)
				: Error(ec, "") {}

			bool IsWarning(void) const {
				return (int)error_code >= 200;
			}
			bool IsMessage(void) const {
				return (int)error_code >= 600;
			}
			bool IsGeneralError(void) const {
				return (int)error_code >= 190 && (int)error_code < 200;
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
		std::list<Error> messages;
		MessageLevel message_level;

		const char* MapMessages(const ErrCode& ec) const;
	};
}