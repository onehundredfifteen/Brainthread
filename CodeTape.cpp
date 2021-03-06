#include "CodeTape.h"

CodeTape::CodeTape()
{
  instructions = NULL;
}
CodeTape::CodeTape(std::vector<CodeTape::bt_instruction> *source_code)
{
   Alloc(source_code->size());
   std::copy(source_code->begin(), source_code->end(), this->instructions);	
}

CodeTape::~CodeTape(void)
{
	if(instructions && len > 0)
		delete [] instructions;

	len = 0;
	instructions = NULL;
}

CodeTape::bt_instruction CodeTape::GetInstruction(unsigned int &code_ptr)
{
	return code_ptr == len ? btoUnkown : instructions[code_ptr];
}


void CodeTape::Alloc(unsigned int size)
{
  try
  {
	  instructions = new bt_instruction[size];
  }
  catch (std::bad_alloc& ba)
  {
      throw std::exception("FATAL> CodeTape::Alloc: Failed to allocate the requested code space");
  }
  catch(...)
  {
	  throw std::exception("FATAL> CodeTape::Alloc: Unknown Error");
  }

  len = size;
}



