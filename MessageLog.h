#pragma once

#include <vector>
#include <string>

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

			ecEmptyCode,
			ecIntegrityLost,
			ecFatalError = 190,
			ecUnkownError = 199,

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
		};
		enum MessageLevel
		{
			mlAll,
			mlImportant,
			mlNone
		};

		struct Error
		{
			unsigned int instruction_pos; 
			ErrCode error_code; 

			std::string text;

			Error(ErrCode ec): error_code(ec),instruction_pos(0){}
			Error(ErrCode ec, unsigned int pos):error_code(ec), instruction_pos(pos){}
			Error(ErrCode ec, std::string t):error_code(ec),instruction_pos(0),text(t){}

			bool IsWarning(){ return (int)error_code >= 200;}
			bool IsMessage(){ return (int)error_code >= 600;}
		};

	private: //singleton
		MessageLog();
		MessageLog(const MessageLog &);
		MessageLog& operator=(const MessageLog&);
	    ~MessageLog(void);

	public:
		static MessageLog& GetInstance();

		void SetMessageLevel(MessageLevel message_level);

		void AddMessage(ErrCode e_code, unsigned int pos);
		void AddMessage(ErrCode e_code, std::string t);
		void AddMessage(std::string t);
		void AddInfo(std::string t);
		
		unsigned ErrorsCount(void);
		unsigned WarningsCount(void);
		unsigned MessagesCount(void);
		void GetMessages(void);

	protected:
		std::vector<Error> messages;
		unsigned error_count;
		unsigned warning_count;
		MessageLevel message_level;

		const char * MapMessages(ErrCode &ec) const;
};