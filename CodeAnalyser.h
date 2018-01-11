#pragma once

#include "Parser.h"
#include "MessageLog.h"

#include <vector>

/*
 * Klasa CodeAnalyser
 * Pozwala debugowaæ i optymalizowaæ kod
*/

class CodeAnalyser
{
	public:
		CodeAnalyser(std::vector<CodeTape::bt_instruction> *instructions, bool repair = false);
		~CodeAnalyser(void);

		void Analyse();

		bool isCodeValid(void);
		bool RepairedSomething();

	private:
		Parser::CodeLang language;
		std::vector<CodeTape::bt_instruction> *instructions;

		short typesize;
		bool repair;
		unsigned repaired_issues;

		bool TestForInfiniteLoops(std::vector<CodeTape::bt_instruction>::iterator &it);
		bool TestForFunctionsErrors(std::vector<CodeTape::bt_instruction>::iterator &it);
		bool TestForThreads(std::vector<CodeTape::bt_instruction>::iterator &it);
		bool TestForHeaps(std::vector<CodeTape::bt_instruction>::iterator &it);

		bool TestLoopPerformance(std::vector<CodeTape::bt_instruction>::iterator &it);
		bool TestArithmetics(std::vector<CodeTape::bt_instruction>::iterator &it);
		bool TestRedundantMoves(std::vector<CodeTape::bt_instruction>::iterator &it);
		void TestOpsBeforeFork(std::vector<CodeTape::bt_instruction>::iterator &it);

		bool TestForRepetition(std::vector<CodeTape::bt_instruction>::iterator &it, const CodeTape::bt_operation &op);
		int Evaluate(const std::vector<CodeTape::bt_instruction>::iterator &begin, const std::vector<CodeTape::bt_instruction>::iterator &end) const;
		int EvaluateMoves(const std::vector<CodeTape::bt_instruction>::iterator &begin, const std::vector<CodeTape::bt_instruction>::iterator &end) const;

		short CodeAnalyser::GetLoopLimes(const std::vector<CodeTape::bt_instruction>::iterator &op) const;
		
		void RelinkCommands(const std::vector<CodeTape::bt_instruction>::iterator &start, short n = 1);
		void RelinkCommands(const std::vector<CodeTape::bt_instruction>::iterator &start, const std::vector<CodeTape::bt_instruction>::iterator &end, short n = 1);

		
		bool IsWithinFunction(const std::vector<CodeTape::bt_instruction>::iterator &op) const;

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