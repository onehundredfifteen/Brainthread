#pragma once

#include "CodeTape.h"
#include "MessageLog.h"
#include <fstream>

/*
 * Klasa Parsera
 * Parser analizuje kod, odrzuca zbêdne znaki i przygotowuje go do interpretacji (np. ³aczy intrukcje 
 * pocz¹tku i koñca pêtli, aby szybiej dokonywaæ skoków).
*/

class Parser
{
	public:
		typedef enum
		{
			clBrainThread,
			clBrainFuck,
			clPBrain,
			clBrainFork,
			clBrainLove,
			clAuto, 
			clDefault
		} code_lang;

		Parser(code_lang lang, MessageLog *messages, bool debug_instructions_on = false);
		~Parser(void);

		void Parse(const char * data);
		void Parse(std::ifstream &in);
		

		void GetCode(CodeTape &c); 
		std::vector<CodeTape::bt_instruction> * GetCode(); 

		bool isCodeValid(void);

	private:
		code_lang language;
		//code_lang RecognizeLang(void);
		bool debug_instructions_mode;

		MessageLog *errors;
		std::vector<CodeTape::bt_instruction> precode;

		void Parse(std::vector<char> &source);
	
		bool isValidOperator(char &c);
		bool isValidDebugOperator(char &c);
	    CodeTape::bt_operation MapCharToOperator(char &c);

		unsigned int FindMatchingRightPair(CodeTape::bt_operation op, unsigned int from_pos);
		unsigned int FindMatchingLeftPair(CodeTape::bt_operation op, unsigned int from_pos); 

		unsigned int GetValidPos(std::vector<char>::iterator &pos, std::vector<char>::iterator &begin, unsigned int not_valid_pos);
};