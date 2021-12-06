#pragma once

#include "Parser.h"
#include "MessageLog.h"

#include <vector>
#include <functional>

/*
 * Klasa CodeAnalyser
 * Pozwala debugowaæ i optymalizowaæ kod
*/

typedef std::vector<CodeTape::bt_instruction>::iterator CodeIterator;

typedef std::function<void(CodeIterator &)> RepairFn;
typedef std::function<void(CodeIterator &, CodeIterator &)> RepairFn2;
typedef std::function<void(CodeIterator &, CodeIterator &, int& ,int&)> RepairFn2i2;

class CodeAnalyser
{
	public:
		CodeAnalyser(std::vector<CodeTape::bt_instruction> *instructions);
		~CodeAnalyser(void);

		void Analyse();
		void Repair();

		bool isCodeValid(void);
		bool RepairedSomething();

	protected:
		const Parser::CodeLang language;
		const short typesize;
    
		std::vector<CodeTape::bt_instruction> *instructions;
		unsigned repaired_issues;

		bool TestForInfiniteLoops(CodeIterator &it, const RepairFn & = nullptr, const RepairFn & = nullptr);
		bool TestForFunctionsErrors(CodeIterator &it, const RepairFn2 & = nullptr, const RepairFn & = nullptr);
		bool TestForThreads(CodeIterator &it, const RepairFn2 & = nullptr, const RepairFn2 & = nullptr);
		bool TestForHeaps(CodeIterator &it, const RepairFn2 & = nullptr, const RepairFn2 & = nullptr);

		bool TestLoopPerformance(CodeIterator &it);
		bool TestArithmetics(CodeIterator &it, const RepairFn2i2 & = nullptr);
		bool TestRedundantMoves(CodeIterator &it, const RepairFn2i2 & = nullptr);
		void TestOpsBeforeFork(CodeIterator &it);

		bool TestForRepetition(CodeIterator &it, const RepairFn2 & = nullptr);
		int Evaluate(const CodeIterator &begin, const CodeIterator &end) const;
		int EvaluateMoves(const CodeIterator &begin, const CodeIterator &end) const;

		short CodeAnalyser::GetLoopLimes(const CodeIterator &op) const;
		
		void RelinkCommands(const CodeIterator &start, short n = 1);
		void RelinkCommands(const CodeIterator &start, const CodeIterator &end, short n = 1);

		
		bool IsWithinFunction(const CodeIterator &op) const;

		bool TestLinks();
		
		unsigned int function_calls;
		unsigned int function_limit;
		unsigned int function_def;
		unsigned int forks;
		unsigned int joins;

		bool ignore_arithmetic_test;
		bool ignore_moves_test;

		static bool IsChangingInstruction(const CodeTape::bt_instruction &op);
		static bool IsArithmeticInstruction(const CodeTape::bt_instruction &op);
		static bool IsMoveInstruction(const CodeTape::bt_instruction &op);
		static bool IsLinkedInstruction(const CodeTape::bt_instruction &op);
		static bool IsChangingCellInstruction(const CodeTape::bt_instruction &op);
		static bool IsSharedHeapInstruction(const CodeTape::bt_instruction &op);
		static bool IsFlowChangingInstruction(const CodeTape::bt_instruction &op);
};