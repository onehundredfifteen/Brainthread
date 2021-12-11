#pragma once

#include "Parser.h"
#include "MessageLog.h"

#include <vector>
#include <functional>

/*
 * Klasa CodeAnalyser
 * Pozwala debugowaæ i optymalizowaæ kod
*/


using namespace CodeTape;
typedef std::function<void(TapeIterator &)> RepairFn;
typedef std::function<void(TapeIterator &, TapeIterator &)> RepairFn2;
typedef std::function<void(TapeIterator &, TapeIterator &, int& ,int&)> RepairFn2i2;

class CodeAnalyser
{
	public:
		CodeAnalyser(Tape &instructions);
		~CodeAnalyser(void);

		void Analyse();
		void Repair();

		bool isCodeValid(void);
		bool RepairedSomething();

	protected:
		const Parser::CodeLang language;
		const short typesize;
		CodeTape::Tape &instructions;
		
		unsigned repaired_issues;

		bool TestForInfiniteLoops(TapeIterator &it, const RepairFn & = nullptr, const RepairFn & = nullptr);
		bool TestForFunctionsErrors(TapeIterator &it, const RepairFn2 & = nullptr, const RepairFn & = nullptr);
		bool TestForThreads(TapeIterator &it, const RepairFn2 & = nullptr, const RepairFn2 & = nullptr);
		bool TestForHeaps(TapeIterator &it, const RepairFn2 & = nullptr, const RepairFn2 & = nullptr);

		bool TestLoopPerformance(TapeIterator &it);
		bool TestArithmetics(TapeIterator &it, const RepairFn2i2 & = nullptr);
		bool TestRedundantMoves(TapeIterator &it, const RepairFn2i2 & = nullptr);
		void TestOpsBeforeFork(TapeIterator &it);

		bool TestForRepetition(TapeIterator &it, const RepairFn2 & = nullptr);
		int Evaluate(const TapeIterator &begin, const TapeIterator &end) const;
		int EvaluateMoves(const TapeIterator &begin, const TapeIterator &end) const;

		short CodeAnalyser::GetLoopLimes(const TapeIterator &op) const;
		
		void RelinkCommands(const TapeIterator &start, short n = 1);
		void RelinkCommands(const TapeIterator &start, const TapeIterator &end, short n = 1);

		
		bool IsWithinFunction(const TapeIterator &op) const;

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