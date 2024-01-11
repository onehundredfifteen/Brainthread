#pragma once

#include "Parser.h"
#include "MessageLog.h"

#include <vector>
#include <functional>

namespace BT {

	/*
	 * Klasa CodeAnalyser
	 * Pozwala debugowaæ i optymalizowaæ kod
	*/



	typedef std::function<void(CodeTapeIterator&)> RepairFn;
	typedef std::function<void(CodeTapeIterator&, CodeTapeIterator&)> RepairFn2;
	typedef std::function<void(CodeTapeIterator&, CodeTapeIterator&, int&, int&)> RepairFn2i2;

	class CodeAnalyser
	{
	public:
		CodeAnalyser(ParserBase &parser);
		~CodeAnalyser(void);

		void Analyse();
		void Repair();

		bool isCodeValid(void);
		bool RepairedSomething();

	protected:
		const CodeLang language;
		const short typesize;
		ParserBase& parser;

		unsigned repaired_issues;

		bool TestForInfiniteLoops(CodeTapeIterator& it, const RepairFn & = nullptr, const RepairFn & = nullptr);
		bool TestForFunctionsErrors(CodeTapeIterator& it, const RepairFn2 & = nullptr, const RepairFn & = nullptr);
		bool TestForThreads(CodeTapeIterator& it, const RepairFn2 & = nullptr, const RepairFn2 & = nullptr);
		bool TestForHeaps(CodeTapeIterator& it, const RepairFn2 & = nullptr, const RepairFn2 & = nullptr);

		bool TestLoopPerformance(CodeTapeIterator& it);
		bool TestArithmetics(CodeTapeIterator& it, const RepairFn2i2 & = nullptr);
		bool TestRedundantMoves(CodeTapeIterator& it, const RepairFn2i2 & = nullptr);
		void TestOpsBeforeFork(CodeTapeIterator& it);

		bool TestForRepetition(CodeTapeIterator& it, const RepairFn2 & = nullptr);
		int Evaluate(const CodeTapeIterator& begin, const CodeTapeIterator& end) const;
		int EvaluateMoves(const CodeTapeIterator& begin, const CodeTapeIterator& end) const;

		short CodeAnalyser::GetLoopLimes(const CodeTapeIterator& op) const;

		void RelinkCommands(const CodeTapeIterator& start, short n = 1);
		void RelinkCommands(const CodeTapeIterator& start, const CodeTapeIterator& end, short n = 1);

		bool IsWithinFunction(const CodeTapeIterator& op) const;

		bool TestLinks();

		unsigned int function_calls;
		unsigned int function_limit;
		unsigned int function_def;
		unsigned int forks;
		unsigned int joins;

		bool ignore_arithmetic_test;
		bool ignore_moves_test;

		static bool IsChangingInstruction(const bt_instruction& op);
		static bool IsArithmeticInstruction(const bt_instruction& op);
		static bool IsMoveInstruction(const bt_instruction& op);
		static bool IsLinkableInstruction(const bt_instruction& op);
		static bool IsChangingCellInstruction(const bt_instruction& op);
		static bool IsSharedHeapInstruction(const bt_instruction& op);
		static bool IsFlowChangingInstruction(const bt_instruction& op);
	};
}