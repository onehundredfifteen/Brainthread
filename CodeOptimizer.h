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

	CodeOptimizer(std::list<unsigned int>&, std::vector<CodeTape::bt_instruction>&, coLevel lvl);
	~CodeOptimizer(void);

	void Optimize();
	void RelinkCommands(const TapeIterator& start, short n);
	void RelinkCommands(const TapeIterator& start, const TapeIterator& end, short n);

	static bool isOptimizable(const CodeTape::bt_operation& ins, bool woLoops = true) {
		//without loops
		return (ins == bt_operation::btoMoveLeft || 
			ins == bt_operation::btoMoveRight ||
			ins == bt_operation::btoIncrement || 
			ins == bt_operation::btoDecrement);
	}


private:
	coLevel level;
	std::list<unsigned int>& optimizer_entrypoints;

	bool OptimizeToZeroLoop(TapeIterator &it, const RepairFn2 & = nullptr);
};
