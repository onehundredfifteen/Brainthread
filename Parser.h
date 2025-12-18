#pragma once

#include <string>

#include "Enumdefs.h"
#include "CodeTape.h"
#include "CodeAnalyser.h"

namespace BT {

	/*
	 * Klasa Parsera
	 * Parser analizuje kod, odrzuca zb�dne znaki i przygotowuje go do interpretacji (np. �aczy intrukcje
	 * pocz�tku i ko�ca p�tli, aby szybiej dokonywa� skok�w).
	*/
	class ParserBase {
		friend class CodeAnalyser;
	protected:
		CodeTape instructions;
		bool syntaxValid;
	public:
		const CodeTape& GetInstructions() const {
			return instructions;
		}
		bool IsSyntaxValid() const {
			return syntaxValid && instructions.back().operation == bt_operation::btoEndProgram;
		}
	};

	template <CodeLang Lang, int OLevel>
	class Parser : public ParserBase {
	public:
		Parser(const std::string& source);

	private:
		bool Parse(const std::string& source);

		bool isValidOperator(const char& c) const;
		bool isRepetitionOptimizableOperator(const bt_operation& op) const;

		bt_operation MapCharToOperator(const char& c) const;
		bt_operation MapOperatorToOptimizedOp(const bt_operation& op) const;

		void HandlePragma(const std::string::const_iterator& begin, const std::string::const_iterator& end, const unsigned int err_pos);

		unsigned int GetValidPos(const std::string::const_iterator& pos, const std::string::const_iterator& begin, unsigned int ignore_ins) const;
	};

	
}