#pragma once

#include <vector>
#include <climits> 

class CodeTape
{
	public:
		typedef enum
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
			//other
			btoDecimalRead,
			btoDecimalWrite,

			//inverse loops - brainlove /TODO
			/*btoInvBeginLoop,
			btoInvEndLoop,
			btoBreak,*/

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
		} bt_operation;

		struct bt_instruction
		{
			bt_operation operation;
			unsigned int jump;

			bt_instruction(bt_operation op, unsigned int index): operation(op), jump(index){};
			bt_instruction(bt_operation op): operation(op), jump(UINT_MAX){};
			bt_instruction(): operation(CodeTape::btoUnkown), jump(UINT_MAX){};

			bool NullJump(){return jump == UINT_MAX;}
		};

		
		CodeTape::CodeTape();
		CodeTape(unsigned int size);
		~CodeTape(void);

		bt_instruction GetInstruction(unsigned int &code_ptr);

		void Copy(std::vector<CodeTape::bt_instruction>::iterator &begin, std::vector<CodeTape::bt_instruction>::iterator &end);

	protected:
		bt_instruction * instructions;
		unsigned int len;

		void Alloc(unsigned int size);
		//static const unsigned int len_limit = 104857600; //100 kb
};