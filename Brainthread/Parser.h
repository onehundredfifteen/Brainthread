#pragma once

#include "CodeTape.h"
#include "ParseErrors.h"
#include <fstream>

class Parser
{
	public:
		typedef enum
		{
			clBrainThread,
			clBrainFuck,
			clPBrain,
			clBrainFork,
			clAuto, 
			clDefault
		} code_lang;

		Parser(void);
		~Parser(void);
		

		void Parse(char * data);
		void Parse(std::ifstream &in);
		
		void Debug();
		void Optimize();

		void GetCode(CodeTape &c); 

		bool isCodeValid(void);
		void GetMessages(void);
		unsigned MessageCount(void);

	protected:
		code_lang language;
		//code_lang RecognizeLang(void);
		ParseErrors errors;
		std::vector<CodeTape::bt_instruction> precode;

		void Parse(std::vector<char> &source);
	
		bool isValidOperator(char &c);
	    CodeTape::bt_operation MapCharToOperator(char &c);

		unsigned int FindMatchingRightPair(CodeTape::bt_operation op, unsigned int from_pos);
		unsigned int FindMatchingLeftPair(CodeTape::bt_operation op, unsigned int from_pos); 

		unsigned int GetValidPos(std::vector<char>::iterator &pos, std::vector<char>::iterator &begin, unsigned int not_valid_pos);
};