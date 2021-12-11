#pragma once

#include <vector>
#include <climits> 

namespace CodeTape {

	
	enum class bt_operation
	{
		btoDecrement = 1,
		btoIncrement,
		btoMoveLeft,
		btoMoveRight,
		btoAsciiRead,
		btoAsciiWrite,
		btoBeginLoop,
		btoEndLoop,
		//functions - pbrain
		btoBeginFunction,
		btoEndFunction,
		btoCallFunction,
		//threads - extended brainfork
		btoFork,
		btoJoin,
		btoTerminate,
		//heap
		btoPush,
		btoPop,
		btoSwap,
		btoSharedPop,
		btoSharedPush,
		btoSharedSwap,
		btoSwitchHeap, //non executable command
		//other
		btoDecimalRead,
		btoDecimalWrite,

		//wrapped, optimized instructions
		btoOPT_SetCellToZero,
		btoOPT_NoOperation,

		//debug instructions
		btoDEBUG_SimpleMemoryDump = 100,
		btoDEBUG_MemoryDump,
		btoDEBUG_StackDump,
		btoDEBUG_SharedStackDump,
		btoDEBUG_FunctionsStackDump,
		btoDEBUG_FunctionsDefsDump,
		btoDEBUG_ThreadInfoDump,

		btoInvalid,
		btoUnkown = 0
	};

	struct bt_instruction
	{
		bt_operation operation;
		unsigned int jump;
		unsigned int repetitions;

		bt_instruction(bt_operation op, unsigned int index, unsigned int reps = 1)
			: operation(op), jump(index), repetitions(reps) {};
		bt_instruction(bt_operation op) 
			: operation(op), jump(UINT_MAX), repetitions(1) {};
		bt_instruction() : operation(bt_operation::btoUnkown), jump(UINT_MAX), repetitions(1) {};

		bool NullJump() { return jump == UINT_MAX; }

		/*bt_instruction(bt_operation op, unsigned int index, unsigned int reps = 0)
			: operation(op), jump(index), repetitions(reps) {};
		bt_instruction(bt_operation op): bt_instruction(op, UINT_MAX){};
		bt_instruction(): bt_instruction(bt_instruction::btoUnkown){};*/
	};
	

	typedef std::vector<bt_instruction> Tape;
	typedef Tape::iterator TapeIterator;
}