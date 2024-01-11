#pragma once

#include <vector>
#include <climits> 

namespace BT {
	enum class bt_operation : uint16_t
	{
		btoDecrement = 1,
		btoIncrement,
		btoMoveLeft,
		btoMoveRight,
		btoOPT_Decrement, //optimized 
		btoOPT_Increment,
		btoOPT_MoveLeft,
		btoOPT_MoveRight,
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
		btoDEBUG_Pragma,

		btoInvalid,
		btoEndProgram,
		btoUnkown = 0
	};

	struct bt_instruction
	{
		bt_operation operation;
		unsigned short repetitions;
		unsigned int jump;
		
		bt_instruction(bt_operation op, unsigned int index, unsigned short reps)
			: operation(op), jump(index), repetitions(reps) {};
		bt_instruction(bt_operation op, unsigned int index)
			: bt_instruction(op, index, 1) {};
		bt_instruction(bt_operation op)
			: bt_instruction(op, UINT_MAX) {};
		bt_instruction() 
			: bt_instruction(bt_operation::btoUnkown) {};

		bool IsLinked() const { return jump < UINT_MAX; }
	};
	
	typedef std::vector<bt_instruction> CodeTape;
	typedef CodeTape::iterator CodeTapeIterator;
}