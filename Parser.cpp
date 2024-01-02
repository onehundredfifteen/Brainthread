#include "Parser.h"
#include "MessageLog.h"
#include "CodeOptimizer.h"

#include <cstring>
#include <stack>
#include <algorithm>
#include <iterator>

namespace BT {


	/*
	 * Klasa Parsera
	 * Parser analizuje kod, odrzuca zbêdne znaki i przygotowuje go do interpretacji (np. ³aczy intrukcje
	 * pocz¹tku i koñca pêtli, aby szybiej dokonywaæ skoków). Mo¿e tez debugowaæ i optymalizowaæ kod
	*/
	template <CodeLang Lang, int OLevel>
	Parser<Lang, OLevel>::Parser(std::string& source)
	{
		Parse(source);
	}

	template <CodeLang Lang, int OLevel>
	void Parser<Lang, OLevel>::Parse(std::string& source)
	{
		std::stack<unsigned int> loop_call_stack;
		std::stack<unsigned int> func_call_stack;

		//optimizer
		std::list<unsigned int> optimizer_entrypoint;

		//for loop jumps
		unsigned int ignore_ins = 0;

		//current op
		bt_operation curr_op;
		std::string::iterator _it;
		int reps = 0;

		//next stack op will be executed on shared stack
		bool switchToSharedHeap = false;
		unsigned int switchJump = 0;

		if (source.empty())
		{
			MessageLog::Instance().AddMessage(MessageLog::ecEmptyCode, 0);
			return;
		}

		instructions.reserve(source.size());

		for (std::string::iterator it = source.begin(); it < source.end(); ++it)
		{
			if (isValidOperator(*it))
			{
				curr_op = MapCharToOperator(*it);

				if (curr_op == bt_operation::btoBeginLoop /*|| curr_op == btoInvBeginLoop*/) //slepe wi¹zanie
				{
					if (OLevel > 0 && (optimizer_entrypoint.empty() ||
						instructions[optimizer_entrypoint.back()].operation != bt_operation::btoBeginLoop)) {

						_it = (it + 2);
						if (_it < source.end() &&
							MapCharToOperator(*_it) == bt_operation::btoEndLoop &&
							MapCharToOperator(*--_it) == bt_operation::btoDecrement) {
							instructions.push_back(bt_instruction(bt_operation::btoOPT_SetCellToZero));
							ignore_ins += 2;
							std::advance(it, 2);
						}
						else {
							loop_call_stack.push(GetValidPos(it, source.begin(), ignore_ins));
							instructions.push_back(bt_instruction(curr_op));

							optimizer_entrypoint.push_back(instructions.size() - 1);
						}

					}
					else if (OLevel > 1) {
						loop_call_stack.push(GetValidPos(it, source.begin(), ignore_ins));
						instructions.push_back(bt_instruction(curr_op));
					}
					else {
						loop_call_stack.push(GetValidPos(it, source.begin(), ignore_ins));
						instructions.push_back(bt_instruction(curr_op));
					}
				}
				else if (curr_op == bt_operation::btoEndLoop /*|| curr_op == btoInvEndLoop*/)
				{
					if (loop_call_stack.empty() == false)
					{
						instructions[loop_call_stack.top()].jump = GetValidPos(it, source.begin(), ignore_ins);
						instructions.push_back(bt_instruction(curr_op, loop_call_stack.top()));

						//szukamy [ ( ] )
						if (func_call_stack.empty() == false && loop_call_stack.top() < func_call_stack.top() && func_call_stack.top() < GetValidPos(it, source.begin(), ignore_ins))
						{
							MessageLog::Instance().AddMessage(MessageLog::ecBLOutOfFunctionScope, GetValidPos(it, source.begin(), ignore_ins));
						}

						loop_call_stack.pop();
					}
					else
					{
						MessageLog::Instance().AddMessage(MessageLog::ecUnmatchedLoopBegin, GetValidPos(it, source.begin(), ignore_ins));
					}

				}
				else if (curr_op == bt_operation::btoBeginFunction)
				{
					func_call_stack.push(GetValidPos(it, source.begin(), ignore_ins));
					instructions.push_back(bt_instruction(curr_op));
				}
				else if (curr_op == bt_operation::btoEndFunction)
				{
					if (func_call_stack.empty() == false)
					{
						instructions[func_call_stack.top()].jump = GetValidPos(it, source.begin(), ignore_ins);
						instructions.push_back(bt_instruction(curr_op, func_call_stack.top()));

						//szukamy ( [ ) ]
						if (loop_call_stack.empty() == false && func_call_stack.top() < loop_call_stack.top() && loop_call_stack.top() < GetValidPos(it, source.begin(), ignore_ins))
						{
							MessageLog::Instance().AddMessage(MessageLog::ecELOutOfFunctionScope, GetValidPos(it, source.begin(), ignore_ins));
						}

						func_call_stack.pop();
					}
					else //nie ma nic do œci¹gniêcia - brak odpowiadaj¹cego (
					{
						MessageLog::Instance().AddMessage(MessageLog::ecUnmatchedFunBegin, GetValidPos(it, source.begin(), ignore_ins));
					}

				}
				/*else if(curr_op == btoBreak)//break w brainlove
				{
					std::vector<char>::iterator br_it = std::find(it, source.end(), ']');
					if(it != source.end())
					{
						instructions.push_back(bt_instruction(curr_op, loop_call_stack.top()));
					}
					else
					{
						errors->AddMessage(MessageLog::ecUnmatchedBreak, GetValidPos(it, source.begin(), ignore_ins), line_counter);
					}
				}*/
				else if (switchToSharedHeap &&
					(curr_op == bt_operation::btoPush || curr_op == bt_operation::btoPop || curr_op == bt_operation::btoSwap))
				{
					switchToSharedHeap = false;

					switch (curr_op)
					{
					case bt_operation::btoPush: instructions.push_back(bt_instruction(bt_operation::btoSharedPush));
						break;
					case bt_operation::btoPop: instructions.push_back(bt_instruction(bt_operation::btoSharedPop));
						break;
					case bt_operation::btoSwap: instructions.push_back(bt_instruction(bt_operation::btoSharedSwap));
						break;
					}

					instructions.back().jump = switchJump;
				}
				else if (curr_op == bt_operation::btoSwitchHeap)
				{
					switchToSharedHeap = true; //non executable operation
					switchJump = GetValidPos(it, source.begin(), ignore_ins);
					//instructions.push_back(bt_operation::bt_instruction(curr_op));
				}
				else if (OLevel > 0 && CodeOptimizer::isOptimizable(curr_op)) {
					reps = 1;
					_it = (it + 1);
					while (_it < source.end()) {
						if (MapCharToOperator(*_it) == curr_op) {
							++_it;
							++reps;
						}
						else break;
					}
					it = (_it - 1);
					ignore_ins += (reps - 1);

					instructions.push_back(bt_instruction(curr_op, UINT_MAX, reps));
				}
				else
					instructions.push_back(bt_instruction(curr_op));
			}
			else
			{
				++ignore_ins;
			}
		}

		//analiza b³edów z liczników
		if (loop_call_stack.empty() == false) //jest coœ do œci¹gniêcia - brak odpowiadaj¹cego ]
		{
			while (loop_call_stack.empty() == false)
			{
				MessageLog::Instance().AddMessage(MessageLog::ecUnmatchedLoopEnd, loop_call_stack.top());
				loop_call_stack.pop();
			}
		}

		if (func_call_stack.empty() == false) //jest coœ do œci¹gniêcia - brak odpowiadaj¹cego )
		{
			while (func_call_stack.empty() == false)
			{
				MessageLog::Instance().AddMessage(MessageLog::ecUnmatchedFunEnd, func_call_stack.top());
				func_call_stack.pop();
			}
		}

		if constexpr (OLevel > 1) {
			CodeOptimizer optimizer(optimizer_entrypoint, instructions, coLevel::co1);
			optimizer.Optimize();
		}
		//koniec
	}


	template <CodeLang Lang, int OLevel>
	bool inline Parser<Lang, OLevel>::isValidOperator(const char& c) const {

		if constexpr (Lang == CodeLang::clBrainThread) {
			if constexpr (OLevel == 0) return strchr("<>+-.,[]()*{}!&^%~;:MTFSEDH", c) != 0;
			return strchr("<>+-.,[]()*{}!&^%~;:", c) != 0;
		}
		else if constexpr (Lang == CodeLang::clPBrain) {
			if constexpr (OLevel == 0) return strchr("<>+-.,[]()*{}!&^%~;:MFED", c) != 0;
			return strchr("<>+-.,[]()*{}!&^%~;:", c) != 0;
		}
		else if constexpr (Lang == CodeLang::clBrainFork) {
			if constexpr (OLevel == 0) return strchr("<>+-.,[]YMTD", c) != 0;
			return strchr("<>+-.,[]Y", c) != 0;
		}
		else {
			if constexpr (OLevel == 0) return strchr("<>+-.,[]#MD", c) != 0;
			return strchr("<>+-.,[]", c) != 0;
		}
	}

	template <CodeLang Lang, int OLevel>
	bt_operation Parser<Lang, OLevel>::MapCharToOperator(const char& c) const
	{
		if constexpr (Lang == CodeLang::clBrainThread) {
			switch (c) {
				case '<': return bt_operation::btoMoveLeft;
				case '>': return bt_operation::btoMoveRight;
				case '+': return bt_operation::btoIncrement;
				case '-': return bt_operation::btoDecrement;
				case '.': return bt_operation::btoAsciiWrite;
				case ',': return bt_operation::btoAsciiRead;
				case '[': return bt_operation::btoBeginLoop;
				case ']': return bt_operation::btoEndLoop;

				case '{': return bt_operation::btoFork;
				case '}': return bt_operation::btoJoin;
				case '!': return bt_operation::btoTerminate;

				case '(': return bt_operation::btoBeginFunction;
				case ')': return bt_operation::btoEndFunction;
				case '*': return bt_operation::btoCallFunction;

				case '&': return bt_operation::btoPush;
				case '^': return bt_operation::btoPop;
				case '%': return bt_operation::btoSwap;
				case '~': return bt_operation::btoSwitchHeap;
					//case '/': return bt_operation::btoSharedPush;
					//case '\\':return bt_operation::btoSharedPop;
					//case '@': return bt_operation::btoSharedSwap;

				case ':': return bt_operation::btoDecimalWrite;
				case ';': return bt_operation::btoDecimalRead;
			}
		}
		else if constexpr (Lang == CodeLang::clBrainFuck) {
			switch (c) {
				case '<': return bt_operation::btoMoveLeft;
				case '>': return bt_operation::btoMoveRight;
				case '+': return bt_operation::btoIncrement;
				case '-': return bt_operation::btoDecrement;
				case '.': return bt_operation::btoAsciiWrite;
				case ',': return bt_operation::btoAsciiRead;
				case '[': return bt_operation::btoBeginLoop;
				case ']': return bt_operation::btoEndLoop;
			}
		}
		else if constexpr (Lang == CodeLang::clPBrain) {
			switch (c) {
				case '<': return bt_operation::btoMoveLeft;
				case '>': return bt_operation::btoMoveRight;
				case '+': return bt_operation::btoIncrement;
				case '-': return bt_operation::btoDecrement;
				case '.': return bt_operation::btoAsciiWrite;
				case ',': return bt_operation::btoAsciiRead;
				case '[': return bt_operation::btoBeginLoop;
				case ']': return bt_operation::btoEndLoop;
				case '(': return bt_operation::btoBeginFunction;
				case ')': return bt_operation::btoEndFunction;
				case ':': return bt_operation::btoCallFunction;
			}
		}
		else if constexpr (Lang == CodeLang::clBrainFork) {
			switch (c) {
				case '<': return bt_operation::btoMoveLeft;
				case '>': return bt_operation::btoMoveRight;
				case '+': return bt_operation::btoIncrement;
				case '-': return bt_operation::btoDecrement;
				case '.': return bt_operation::btoAsciiWrite;
				case ',': return bt_operation::btoAsciiRead;
				case '[': return bt_operation::btoBeginLoop;
				case ']': return bt_operation::btoEndLoop;
				case 'Y': return bt_operation::btoFork;
			}
		}

		if constexpr (OLevel == 0) //debug 
		{
			if constexpr (Lang == CodeLang::clBrainThread) {
				switch (c) {
					case 'M': return bt_operation::btoDEBUG_SimpleMemoryDump;
					case 'D': return bt_operation::btoDEBUG_MemoryDump;
					case 'F': return bt_operation::btoDEBUG_FunctionsStackDump;
					case 'E': return bt_operation::btoDEBUG_FunctionsDefsDump;
					case 'S': return bt_operation::btoDEBUG_StackDump;
					case 'H': return bt_operation::btoDEBUG_SharedStackDump;
					case 'T': return bt_operation::btoDEBUG_ThreadInfoDump;
				}
			}
			else if constexpr (Lang == CodeLang::clBrainFuck) {
				switch (c) {
					case '#':
					case 'M': return bt_operation::btoDEBUG_SimpleMemoryDump;
					case 'D': return bt_operation::btoDEBUG_MemoryDump;
				}
			}
			else if constexpr (Lang == CodeLang::clPBrain) {
				switch (c) {
					case '#':
					case 'M': return bt_operation::btoDEBUG_SimpleMemoryDump;
					case 'D': return bt_operation::btoDEBUG_MemoryDump;
					case 'F': return bt_operation::btoDEBUG_FunctionsStackDump;
					case 'E': return bt_operation::btoDEBUG_FunctionsDefsDump;
				}
			}
			else if constexpr (Lang == CodeLang::clBrainFork) {
				switch (c) {
					case '#':
					case 'M': return bt_operation::btoDEBUG_SimpleMemoryDump;
					case 'D': return bt_operation::btoDEBUG_MemoryDump;
					case 'T': return bt_operation::btoDEBUG_ThreadInfoDump;
				}
			}
		}

		return bt_operation::btoInvalid;
	}

	template <CodeLang Lang, int OLevel>
	unsigned int inline Parser<Lang, OLevel>::GetValidPos(const std::string::iterator& pos, const std::string::iterator& begin, unsigned int& not_valid_pos) const
	{
		return pos - begin - not_valid_pos;
	}

	// Explicit template instantiation
	template class Parser<CodeLang::clBrainThread, 0>;
	template class Parser<CodeLang::clBrainThread, 1>;
	template class Parser<CodeLang::clBrainThread, 2>;
	template class Parser<CodeLang::clPBrain, 0>;
	template class Parser<CodeLang::clPBrain, 1>;
	template class Parser<CodeLang::clPBrain, 2>;
	template class Parser<CodeLang::clBrainFork, 0>;
	template class Parser<CodeLang::clBrainFork, 1>;
	template class Parser<CodeLang::clBrainFork, 2>;
	template class Parser<CodeLang::clBrainFuck, 0>;
	template class Parser<CodeLang::clBrainFuck, 1>;
	template class Parser<CodeLang::clBrainFuck, 2>;
}

