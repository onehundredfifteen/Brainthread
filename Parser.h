#pragma once

#include "CodeTape.h"
#include <fstream>

namespace BT {

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
		clBrainLove
	};


	class ParserBase {
	protected:
		CodeTape instructions;

	public:
		CodeTape GetInstructions() {
			return instructions;
		}
	};


	template <CodeLang Lang, int OLevel>
	class Parser : public ParserBase {
	public:
		Parser(std::string& source);

		

	private:
		

		void Parse(std::string& source);

		bool isValidOperator(const char& c) const;
		bt_operation MapCharToOperator(const char& c) const;

		unsigned int GetValidPos(const std::string::iterator& pos, const std::string::iterator& begin, unsigned int& not_valid_pos) const;
	};

	
}