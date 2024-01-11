#pragma once

#include <string>

#include "EnumDefs.h"
#include "CodeTape.h"


namespace BT {

	/*
	 * Klasa Parsera
	 * Parser analizuje kod, odrzuca zbêdne znaki i przygotowuje go do interpretacji (np. ³aczy intrukcje
	 * pocz¹tku i koñca pêtli, aby szybiej dokonywaæ skoków).
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

		bool HandlePragma(const std::string::const_iterator& pos, const std::string::const_iterator& end, unsigned int &ignore_ins);

		unsigned int GetValidPos(const std::string::const_iterator& pos, const std::string::const_iterator& begin, unsigned int ignore_ins) const;
	};

	
}