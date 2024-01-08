#include "Parser.h"
#include "MessageLog.h"

#include <cstring>
#include <stack>
#include <list>
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
		syntaxValid = Parse(source);
	}

	template <CodeLang Lang, int OLevel>
	bool Parser<Lang, OLevel>::Parse(std::string& source)
	{
		std::stack<unsigned int> loop_call_stack;
		std::stack<unsigned int> func_call_stack;

		//optimizer
		std::list<unsigned int> optimizer_entrypoint;

		//non-code instruction count
		unsigned int ignore_ins = 0;

		//next stack op will be executed on shared stack
		bool switchToSharedHeap = false;

		//result
		bool syntaxOk = true;

		if (source.empty())
		{
			MessageLog::Instance().AddMessage(MessageLog::ecEmptyCode, 0);
			return false;
		}

		instructions.reserve(source.size());

		for (std::string::iterator it = source.begin(); it < source.end(); ++it)
		{
			if (isValidOperator(*it))
			{
				bt_operation curr_op = MapCharToOperator(*it);

				if (curr_op == bt_operation::btoBeginLoop /*|| curr_op == btoInvBeginLoop*/)
				{
					if constexpr (OLevel > 1) {
						//optimize [-] to :=0
						std::string::iterator _it = (it + 2);
						if (_it < source.end() &&
							MapCharToOperator(*_it) == bt_operation::btoEndLoop &&
							MapCharToOperator(*--_it) == bt_operation::btoDecrement) {
								instructions.emplace_back(bt_instruction(bt_operation::btoOPT_SetCellToZero));
								ignore_ins += 2;
								std::advance(it, 2);
						}
						else {
							loop_call_stack.push(GetValidPos(it, source.begin(), ignore_ins));
							instructions.emplace_back(bt_instruction(curr_op));

							//optimizer_entrypoint.push_back(instructions.size() - 1);
						}
					}				
					else {
						loop_call_stack.push(GetValidPos(it, source.begin(), ignore_ins));
						instructions.emplace_back(bt_instruction(curr_op));
					}					
				}
				else if (curr_op == bt_operation::btoEndLoop /*|| curr_op == btoInvEndLoop*/)
				{
					if (loop_call_stack.empty() == false)
					{
						instructions[loop_call_stack.top()].jump = GetValidPos(it, source.begin(), ignore_ins);
						instructions.emplace_back(bt_instruction(curr_op, loop_call_stack.top()));

						//let's find [ ( ] )
						if (func_call_stack.empty() == false && loop_call_stack.top() < func_call_stack.top() && func_call_stack.top() < GetValidPos(it, source.begin(), ignore_ins))
						{
							MessageLog::Instance().AddMessage(MessageLog::ecBLOutOfFunctionScope, GetValidPos(it, source.begin(), ignore_ins));
							syntaxOk = false;
						}

						loop_call_stack.pop();
					}
					else
					{
						MessageLog::Instance().AddMessage(MessageLog::ecUnmatchedLoopBegin, GetValidPos(it, source.begin(), ignore_ins));
						syntaxOk = false;
					}
				}
				else if (curr_op == bt_operation::btoBeginFunction)
				{
					func_call_stack.push(GetValidPos(it, source.begin(), ignore_ins));
					instructions.emplace_back(bt_instruction(curr_op));
				}
				else if (curr_op == bt_operation::btoEndFunction)
				{
					if (func_call_stack.empty() == false)
					{
						instructions[func_call_stack.top()].jump = GetValidPos(it, source.begin(), ignore_ins);
						instructions.emplace_back(bt_instruction(curr_op, func_call_stack.top()));

						//let's find ( [ ) ]
						if (loop_call_stack.empty() == false && func_call_stack.top() < loop_call_stack.top() && loop_call_stack.top() < GetValidPos(it, source.begin(), ignore_ins))
						{
							MessageLog::Instance().AddMessage(MessageLog::ecELOutOfFunctionScope, GetValidPos(it, source.begin(), ignore_ins));
							syntaxOk = false;
						}

						func_call_stack.pop();
					}
					else //error, no matching (
					{
						MessageLog::Instance().AddMessage(MessageLog::ecUnmatchedFunBegin, GetValidPos(it, source.begin(), ignore_ins));
						syntaxOk = false;
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
						case bt_operation::btoPush: instructions.emplace_back(bt_instruction(bt_operation::btoSharedPush)); break;
						case bt_operation::btoPop: instructions.emplace_back(bt_instruction(bt_operation::btoSharedPop)); break;
						case bt_operation::btoSwap: instructions.emplace_back(bt_instruction(bt_operation::btoSharedSwap)); break;
					}
				}
				else if (curr_op == bt_operation::btoSwitchHeap)
				{
					switchToSharedHeap = true; 
					++ignore_ins; //non executable operation
				}			
				else if constexpr (OLevel > 1) {
					if (isRepetitionOptimizableOperator(curr_op)) {
						int reps = 1;
						std::string::iterator _it = (it + 1);
						while (_it < source.end()) {
							if (MapCharToOperator(*_it) == curr_op) {
								++_it;
								++reps;
							}
							else break;
						}
						it = (_it - 1);
						ignore_ins += (reps - 1);

						instructions.emplace_back(bt_instruction(MapOperatorToOptimizedOp(curr_op), UINT_MAX, reps));
					}
					else instructions.emplace_back(bt_instruction(curr_op));
				}
				else
					instructions.emplace_back(bt_instruction(curr_op));
			}
			else
			{
				++ignore_ins;
			}
		}

		//error analysis
		while (loop_call_stack.empty() == false){
			MessageLog::Instance().AddMessage(MessageLog::ecUnmatchedLoopEnd, loop_call_stack.top());
			loop_call_stack.pop();
			syntaxOk = false;	
		}

		while (func_call_stack.empty() == false){
			MessageLog::Instance().AddMessage(MessageLog::ecUnmatchedFunEnd, func_call_stack.top());
			func_call_stack.pop();
			syntaxOk = false;
		}

		instructions.emplace_back(bt_instruction(bt_operation::btoEndProgram));

		return syntaxOk;
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
	bool inline Parser<Lang, OLevel>::isRepetitionOptimizableOperator(const bt_operation& op) const { 
		return (op == bt_operation::btoMoveLeft ||
			op == bt_operation::btoMoveRight ||
			op == bt_operation::btoIncrement ||
			op == bt_operation::btoDecrement);
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
	bt_operation Parser<Lang, OLevel>::MapOperatorToOptimizedOp(const bt_operation& op) const {
		if constexpr (OLevel <= 1) {
			return op;
		}
		switch (op) {
			case bt_operation::btoMoveLeft: return bt_operation::btoOPT_MoveLeft;
			case bt_operation::btoMoveRight: return bt_operation::btoOPT_MoveRight;
			case bt_operation::btoIncrement: return bt_operation::btoOPT_Increment;
			case bt_operation::btoDecrement: return bt_operation::btoOPT_Decrement;
			default: return op;
		}
	}

	template <CodeLang Lang, int OLevel>
	unsigned int inline Parser<Lang, OLevel>::GetValidPos(const std::string::iterator& pos, const std::string::iterator& begin, unsigned int not_valid_pos) const
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

