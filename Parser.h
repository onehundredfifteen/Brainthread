#pragma once

#include "CodeTape.h"
#include <fstream>

/*
 * Klasa Parsera
 * Parser analizuje kod, odrzuca zb�dne znaki i przygotowuje go do interpretacji (np. �aczy intrukcje 
 * pocz�tku i ko�ca p�tli, aby szybiej dokonywa� skok�w).
*/

enum class CodeLang
{
	clBrainThread,
	clBrainFuck,
	clPBrain,
	clBrainFork,
	//clBrainLove,
	clAuto,
	clDefault
};


template < int OptL >
class Parser
{
	public:
		Parser(CodeLang lang);

		void Parse(const char * data);
		void Parse(std::ifstream &in);
		
		bool isCodeValid(void) const;
		//releases ownerhip
		CodeTape::Tape GetCode(); 

	private:
		CodeTape::Tape instructions;
		CodeLang language;

		void Parse(std::vector<char> &source);
		CodeLang RecognizeLang(std::vector<char> &source) const;
	
		bool isValidOperator(const char &c) const;
		bool isValidDebugOperator(const char &c) const;
	    CodeTape::bt_operation MapCharToOperator(const char &c) const;

		unsigned int GetValidPos(const std::vector<char>::iterator &pos, const std::vector<char>::iterator &begin, unsigned int &not_valid_pos) const;
};

