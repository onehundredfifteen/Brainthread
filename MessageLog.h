#pragma once

#include <vector>
#include <string>

class MessageLog
{
	public:
		typedef enum 
		{
			ecUnmatchedLoopBegin = 100,
			ecUnmatchedLoopEnd,
			ecUnmatchedFunBegin,
			ecUnmatchedFunEnd,
			ecBLOutOfFunctionScope,
			ecELOutOfFunctionScope,
			ecUnmatchedBreak,

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


			ecMessage = 600,
			ecInformation //taka wiadomoœæ bêdzie pomijana
		} ErrCode;

		typedef enum 
		{
			mlAll,
			mlImportant,
			mlNone
		} MessageLevel;

		struct Error
		{
			unsigned int instruction_pos; 
			ErrCode error_code; 

			std::string text;

			Error(ErrCode ec): error_code(ec),instruction_pos(0){}
			Error(ErrCode ec, unsigned int pos):error_code(ec), instruction_pos(pos){}
			Error(ErrCode ec, std::string t):error_code(ec), instruction_pos(0), text(t){}

			bool IsWarning(void) { 
				return (int)error_code >= 200;
			}
			bool IsMessage(void) { 
				return (int)error_code >= 600;
			}
			bool IsGeneralError(void) { 
				return (int)error_code >= 190 && (int)error_code < 200;
			}
		};

	public:
		static MessageLog & Instance()
		{
			static MessageLog instance;

			return instance;
		}

		void SetMessageLevel(MessageLevel message_level);

		void AddMessage(ErrCode e_code, unsigned int pos);
		void AddMessage(ErrCode e_code, std::string t);
		void AddMessage(std::string t);
		void AddInfo(std::string t);
		
		unsigned ErrorsCount(void);
		unsigned WarningsCount(void);
		unsigned MessagesCount(void);
		void GetMessages(void);

	private: 
		MessageLog() {
			error_count = 0;
			warning_count = 0;
		}
		~MessageLog(void){};

		MessageLog(MessageLog const&);            
		MessageLog& operator=(MessageLog const&);  

	protected:
		std::vector<Error> messages;
		unsigned error_count;
		unsigned warning_count;
		MessageLevel message_level;

		const char * MapMessages(ErrCode &ec) const;
};