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
			ecFatalError = 190,
			ecUnkownError = 199,

			//warningi
			ecInfiniteLoop = 200,
			ecEmptyLoop,
			ecEmptyFunction,
			ecFunctionRedefinition,
			ecFunctionRedefinition2,

			ecFunctionLimitExceed,
			ecFunctionExistsButNoCall,
			ecJoinButNoFork,
			ecTerminateRepeat,
			ecJoinRepeat,

			ecJoinBeforeFork,
			ecRedundantArithmetic,
			ecRedundantArithmetic2,
			ecRedundantLoopArithmetic,
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
			unsigned int line; 
			unsigned int col; 
			ErrCode error_code; 

			std::string text;

			Error(ErrCode ec):error_code(ec),line(0),col(0){}
			Error(ErrCode ec, unsigned int c):error_code(ec),line(0),col(c){}
			Error(ErrCode ec, unsigned int c, unsigned int l):error_code(ec),line(l),col(c){}
			Error(ErrCode ec, std::string t):error_code(ec),line(0),col(0),text(t){}

			bool IsWarning(){ return (int)error_code >= 200;}
			bool IsMessage(){ return (int)error_code >= 600;}
		};

		MessageLog(MessageLevel message_level);
	    ~MessageLog(void);

		void AddMessage(ErrCode e_code, unsigned int pos, unsigned int line = 1);
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