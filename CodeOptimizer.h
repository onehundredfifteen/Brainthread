#pragma once

#include "CodeAnalyser.h"
#include <list>
/*
 * Klasa CodeOptimizer
 * Pozwala optymalizowaæ kod
*/

namespace BT {

	typedef enum
	{
		co1 = 1,
		co2,
		none
	} coLevel;

	class CodeOptimizer : public CodeAnalyser
	{
	public:

		CodeOptimizer(std::list<unsigned int>&, std::vector<bt_instruction>&, coLevel lvl);
		~CodeOptimizer(void);

		void Optimize();
		void RelinkCommands(const CodeTapeIterator& start, short n);
		void RelinkCommands(const CodeTapeIterator& start, const CodeTapeIterator& end, short n);

		


	private:
		coLevel level;
		std::list<unsigned int>& optimizer_entrypoints;

		bool OptimizeToZeroLoop(CodeTapeIterator& it, const RepairFn2 & = nullptr);
	};
}