#pragma once

#include "CodeAnalyser.h"
#include <queue>
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

	CodeOptimizer(std::vector<CodeTape::bt_instruction> *instructions, coLevel lvl);
	~CodeOptimizer(void);

	void Optimize(std::queue<unsigned int> &optimizer_entrypoints);


private:
	coLevel level;

	bool OptimizeToZeroLoop(CodeIterator &it, const RepairFn2 & = nullptr);
};
