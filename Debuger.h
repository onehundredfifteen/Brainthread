#pragma once

#include "Parser.h"
#include "MessageLog.h"

#include <vector>

/*
 * Klasa Debugera
 * debugowaæ i optymalizowaæ kod
*/

class Debuger
{
	public:
		Debuger(std::vector<CodeTape::bt_instruction> *instructions, bool repair = false);
		~Debuger(void);

		void Debug();

		bool isCodeValid(void);

	private:
		Parser::code_lang language;
		std::vector<CodeTape::bt_instruction> *instructions;

		short typesize;
		bool repair;
		unsigned repaired_issues;

		bool TestForInfiniteLoops(std::vector<CodeTape::bt_instruction>::iterator &it);
		bool TestForFunctionsErrors(std::vector<CodeTape::bt_instruction>::iterator &it);
		bool TestThreads(std::vector<CodeTape::bt_instruction>::iterator &it);

		bool TestLoopPerformance(std::vector<CodeTape::bt_instruction>::iterator &it);
		bool TestArithmetics(std::vector<CodeTape::bt_instruction>::iterator &it);
		bool TestRedundantMoves(std::vector<CodeTape::bt_instruction>::iterator &it);
		void TestOpsBeforeFork(std::vector<CodeTape::bt_instruction>::iterator &it);

		bool TestForRepetition(std::vector<CodeTape::bt_instruction>::iterator &it, const CodeTape::bt_operation &op);
		int Calcule(const std::vector<CodeTape::bt_instruction>::iterator &begin, const std::vector<CodeTape::bt_instruction>::iterator &end) const;
		int CalculeMoves(const std::vector<CodeTape::bt_instruction>::iterator &begin, const std::vector<CodeTape::bt_instruction>::iterator &end) const;

		short Debuger::GetLoopLimes(const std::vector<CodeTape::bt_instruction>::iterator &op) const;
		
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
};