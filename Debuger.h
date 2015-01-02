#pragma once

#include "CodeTape.h"
#include "ParseErrors.h"

#include <vector>

/*
 * Klasa Debugera
 * debugowaæ i optymalizowaæ kod
*/

class Debuger
{
	public:
		Debuger(ParseErrors *messages, std::vector<CodeTape::bt_instruction> *precode, bool repair = false);
		~Debuger(void);

		void Debug();

		bool isCodeValid(void);

	private:
		CodeTape::code_lang language;
		std::vector<CodeTape::bt_instruction> *precode;

		ParseErrors *messages;
		bool repair;
		unsigned repaired_issues;

		void TestForInfiniteLoops(std::vector<CodeTape::bt_instruction>::iterator &it);
		void TestForLoopsOutOfScope(std::vector<CodeTape::bt_instruction>::iterator &it);
		void TestForFunctionsErrors(std::vector<CodeTape::bt_instruction>::iterator &it);
		bool TestForRepetition(std::vector<CodeTape::bt_instruction>::iterator &it, const CodeTape::bt_operation &op);
		void TestForJoinBeforeFork(std::vector<CodeTape::bt_instruction>::iterator &it);

		void TestArithmetics(std::vector<CodeTape::bt_instruction>::iterator &it);
		void TestArithmeticsLoops(std::vector<CodeTape::bt_instruction>::iterator &it);
		void TestRedundantMoves(std::vector<CodeTape::bt_instruction>::iterator &it);

		int Calcule(std::vector<CodeTape::bt_instruction>::iterator &begin, std::vector<CodeTape::bt_instruction>::iterator &end);
		int CalculeMoves(std::vector<CodeTape::bt_instruction>::iterator &begin, std::vector<CodeTape::bt_instruction>::iterator &end);
		
		void RelinkCommands(std::vector<CodeTape::bt_instruction>::iterator &start, unsigned n = 1);

		bool ARSearchTool(std::vector<CodeTape::bt_instruction>::iterator &begin);

		unsigned int function_calls;
		unsigned int function_limit;
		unsigned int function_def;
		unsigned int forks;
		unsigned int joins;

		bool ignore_arithmetic_for_error;
		bool ignore_moves_for_error;

		static bool IsChangingInstruction(const CodeTape::bt_instruction &op);
		static bool IsArithmeticInstruction(const CodeTape::bt_instruction &op);
		static bool IsArithmeticSafeInstruction(const CodeTape::bt_instruction &op);
		static bool IsMoveInstruction(const CodeTape::bt_instruction &op);
		static bool IsMoveSafeInstruction(const CodeTape::bt_instruction &op);
};