#pragma once

#include "CodeAnalyser.h"
#include <list>
/*
 * Klasa CodeOptimizer
 * Pozwala optymalizowaæ kod
*/

typedef enum
{
	co1 = 1,
	co2,
	none
} coLevel;

class CodeOptimizer : public CodeAnalyser
{
public:

	CodeOptimizer(std::list<unsigned int>&, std::vector<CodeTape::bt_instruction> *, coLevel lvl);
	~CodeOptimizer(void);

	void Optimize();
	void RelinkCommands(const CodeIterator& start, short n);
	void RelinkCommands(const CodeIterator& start, const CodeIterator& end, short n);

	static bool isOptimizable(const CodeTape::bt_operation& ins, bool woLoops = true) {
		//without loops
		return (ins == CodeTape::btoMoveLeft || 
			ins == CodeTape::btoMoveRight ||
			ins == CodeTape::btoIncrement || 
			ins == CodeTape::btoDecrement);
	}


private:
	coLevel level;
	std::list<unsigned int>& optimizer_entrypoints;

	bool OptimizeToZeroLoop(CodeIterator &it, const RepairFn2 & = nullptr);
};
