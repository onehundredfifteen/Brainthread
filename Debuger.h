#pragma once

#include "CodeTape.h"
#include "MessageLog.h"

#include <vector>

/*
 * Klasa Debugera
 * debugowaæ i optymalizowaæ kod
*/

class Debuger
{
	public:
		Debuger(MessageLog *messages, std::vector<CodeTape::bt_instruction> *precode, short typesize, bool repair = false);
		~Debuger(void);

		void Debug();

		bool isCodeValid(void);

	private:
		CodeTape::code_lang language;
		std::vector<CodeTape::bt_instruction> *precode;

		MessageLog *messages;
		short typesize;
		bool repair;
		unsigned repaired_issues;

		void TestForInfiniteLoops(std::vector<CodeTape::bt_instruction>::iterator &it);
		void TestForFunctionsErrors(std::vector<CodeTape::bt_instruction>::iterator &it);
		bool TestForRepetition(std::vector<CodeTape::bt_instruction>::iterator &it, const CodeTape::bt_operation &op);
		void TestForJoinBeforeFork(std::vector<CodeTape::bt_instruction>::iterator &it);

		void TestArithmetics(std::vector<CodeTape::bt_instruction>::iterator &it);
		void TestArithmeticsLoops(std::vector<CodeTape::bt_instruction>::iterator &it);
		void TestRedundantMoves(std::vector<CodeTape::bt_instruction>::iterator &it);

		int Calcule(const std::vector<CodeTape::bt_instruction>::iterator &begin, const std::vector<CodeTape::bt_instruction>::iterator &end) const;
		int CalculeMoves(const std::vector<CodeTape::bt_instruction>::iterator &begin, const std::vector<CodeTape::bt_instruction>::iterator &end) const;
		
		void RelinkCommands(const std::vector<CodeTape::bt_instruction>::iterator &start, short n = 1);
		void RelinkCommands(const std::vector<CodeTape::bt_instruction>::iterator &start, const std::vector<CodeTape::bt_instruction>::iterator &end, short n = 1);

		bool ARSearchTool(const std::vector<CodeTape::bt_instruction>::iterator &begin);
		
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
		static bool IsLinkedInstruction(const CodeTape::bt_instruction &op);
};