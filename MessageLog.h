#pragma once

#include <vector>
#include <string>

class MessageLog
{
	public:
		enum ErrCode
		{
			ecLoopUnmatchedR = 100,
			ecLoopUnmatchedL,
			ecUnmatchedFunBegin,
			ecUnmatchedFunEnd,
			ecBLOutOfFunctionScope,
			ecELOutOfFunctionScope,

			ecFatalError = 190,
			ecEmptyCode,
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
			ecRedundantLoopArithmetic,
			ecRedundantNearLoopArithmetic,
			ecSlowLoop,

			ecRedundantMoves,

			ecMessage = 600,
			ecInformation //taka wiadomoœæ bêdzie pomijana
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

		MessageLog(bool iom);
	    ~MessageLog(void);

		void AddMessage(ErrCode e_code, unsigned int pos, unsigned int line = 1);
		void AddMessage(ErrCode e_code, std::string t);
		
		unsigned ErrorsCount(void);
		unsigned WarningsCount(void);
		unsigned MessagesCount(void);
		void GetMessages(void);

	protected:
		std::vector<Error> messages;
		unsigned error_count;
		unsigned warning_count;
		bool important_messages_only;

		const char * MapMessages(ErrCode &ec) const;
};