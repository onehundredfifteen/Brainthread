#pragma once

#include <vector>

class ParseErrors
{
	public:
		enum ErrCode
		{
			ecLoopUnmatchedR = 100,
			ecLoopUnmatchedL,
			ecUnmatchedFunBegin,
			ecUnmatchedFunEnd,

			ecFatalError = 190,
			ecEmptyCode,
			ecUnkownError = 199,

			//warningi
			ecInfiniteLoop = 200
		};

		struct Error
		{
			unsigned int line; 
			unsigned int col; 
			ErrCode error_code; 

			Error(ErrCode ec):error_code(ec),line(1),col(0){}
			Error(ErrCode ec, unsigned int l, unsigned int c):error_code(ec),line(l),col(c){}

			bool IsWarning(){ return (int)error_code >= 200;}
		};

		ParseErrors(void);
	    ~ParseErrors(void);

		void AddMessage(ErrCode e_code, unsigned int pos, unsigned int line = 1);
		
		unsigned ErrorsCount(void);
		unsigned WarningsCount(void);
		void GetMessages(void);

	protected:
		std::vector<Error> messages;
		unsigned error_count;

		const char * MapMessages(ErrCode &ec) const;
};