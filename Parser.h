#pragma once

#include "CodeTape.h"
#include "MessageLog.h"
#include <fstream>

/*
 * Klasa Parsera
 * Parser analizuje kod, odrzuca zb�dne znaki i przygotowuje go do interpretacji (np. �aczy intrukcje 
 * pocz�tku i ko�ca p�tli, aby szybiej dokonywa� skok�w).
*/

class Parser
{
	public:
		Parser(MessageLog *messages, bool debug_instructions_on = false);
		~Parser(void);

		void Parse(char * data);
		void Parse(std::ifstream &in);
		

		void GetCode(CodeTape &c); 
		std::vector<CodeTape::bt_instruction> * GetCode(); 

		bool isCodeValid(void);

	private:
		CodeTape::code_lang language;
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