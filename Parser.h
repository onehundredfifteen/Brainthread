#pragma once


#include "CodeTape.h"
#include "EnumDefs.h"

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
		Parser(std::string& source);

	private:
		bool Parse(std::string& source);

		bool isValidOperator(const char& c) const;
		bool isRepetitionOptimizableOperator(const bt_operation& op) const;

		bt_operation MapCharToOperator(const char& c) const;
		bt_operation MapOperatorToOptimizedOp(const bt_operation& op) const;

		unsigned int GetValidPos(const std::string::iterator& pos, const std::string::iterator& begin, unsigned int not_valid_pos) const;
	};

	
}