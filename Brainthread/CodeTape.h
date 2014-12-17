#pragma once

#include <vector>
#include <climits> 

class CodeTape
{
	public:
		typedef enum
		{
			btoDecrement,
			btoIncrement,
			btoMoveLeft,
			btoMoveRight,
			btoStdRead,
			btoStdWrite,
			btoBeginLoop,
			btoEndLoop,
			//functions - pbrain
			btoBeginFunction,
			btoEndFunction,
			btoCallFunction,
			//threads - extended brainfork
			btoFork,
			btoWait,
			btoTerminate,
			//heap
			btoPush,
			btoPop,
			btoSwap,
			btoSharedPop,
			btoSharedPush,
			btoSharedSwap,
			//other
			btoNatRead,
			btoNatWrite,

			btoInvalid,
			btoUnkown
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

		bt_instruction ToExecute(unsigned int &code_ptr);

		void CodeTape::Copy(std::vector<CodeTape::bt_instruction>::iterator &begin, std::vector<CodeTape::bt_instruction>::iterator &end);

	protected:
		bt_instruction * instructions;
		
		void CodeTape::Alloc(unsigned int size);


		unsigned int len;
		static const unsigned int len_limit = 104857600; //100 kb
};