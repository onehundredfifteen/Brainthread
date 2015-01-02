#include "Parser.h"

#include <cstring>
#include <stack>

/*
 * Klasa Parsera
 * Parser analizuje kod, odrzuca zbêdne znaki i przygotowuje go do interpretacji (np. ³aczy intrukcje 
 * pocz¹tku i koñca pêtli, aby szybiej dokonywaæ skoków). Mo¿e tez debugowaæ i optymalizowaæ kod
*/

Parser::Parser(ParseErrors *messages, bool debug_instructions_on)
{
	language = CodeTape::clBrainThread;
	
	if(messages == nullptr)
		errors = new ParseErrors;
	else
		errors = messages;

	debug_instructions_mode = debug_instructions_on;
}

Parser::~Parser(void)
{
}


void Parser::Parse(char * data)
{
	std::vector<char> buffer (data, data + strlen(data));
	Parse(buffer);
}

void Parser::Parse(std::ifstream &in)
{
	//std::vector<char> buffer (data, data + strlen(data));
	//Parse(buffer);
}


void Parser::Parse(std::vector<char> &source)
{
  std::stack<unsigned int> loop_call_stack;
  std::stack<unsigned int> func_call_stack;

  unsigned int not_valid_ins = 0;
  CodeTape::bt_operation curr_op;

  unsigned int line_counter = 1;

  if(source.empty())
  {
	  errors->AddMessage(ParseErrors::ecEmptyCode, 0);
	  return;
  }

  precode.reserve(source.size()/2);
  
  for(std::vector<char>::iterator it = source.begin(); it < source.end(); ++it)
  {
	if(isValidOperator(*it) || (debug_instructions_mode && isValidDebugOperator(*it)) )
	{
		curr_op = MapCharToOperator(*it);
		
		if(curr_op == CodeTape::btoBeginLoop) //slepe wi¹zanie
		{
			loop_call_stack.push(GetValidPos(it, source.begin(), not_valid_ins));
			precode.push_back(CodeTape::bt_instruction(curr_op));
		}
		else if(curr_op == CodeTape::btoEndLoop)
		{
			if(loop_call_stack.empty() == false) 
			{
				precode[ loop_call_stack.top() ].jump = GetValidPos(it, source.begin(), not_valid_ins);
				precode.push_back(CodeTape::bt_instruction(curr_op, loop_call_stack.top()));
				loop_call_stack.pop();
			}
			else //nie ma nic do œci¹gniêcia - brak odpowiadaj¹cego [
			{
				errors->AddMessage(ParseErrors::ecLoopUnmatchedR, GetValidPos(it, source.begin(), not_valid_ins), line_counter);
			}
					
		}
		else if(curr_op == CodeTape::btoBeginFunction && (language == CodeTape::clBrainThread || language == CodeTape::clPBrain)) 
		{
			func_call_stack.push(GetValidPos(it, source.begin(), not_valid_ins));
			precode.push_back(CodeTape::bt_instruction(curr_op));
		}
		else if(curr_op == CodeTape::btoEndFunction && (language == CodeTape::clBrainThread || language == CodeTape::clPBrain))
		{
			if(func_call_stack.empty() == false) 
			{
				precode[ func_call_stack.top() ].jump = GetValidPos(it, source.begin(), not_valid_ins);
				precode.push_back(CodeTape::bt_instruction(curr_op, func_call_stack.top()));
				func_call_stack.pop();
			}
			else //nie ma nic do œci¹gniêcia - brak odpowiadaj¹cego (
			{
				errors->AddMessage(ParseErrors::ecUnmatchedFunBegin, GetValidPos(it, source.begin(), not_valid_ins), line_counter);
			}
					
		}
		else precode.push_back(CodeTape::bt_instruction(curr_op));
	}
	else
	{	
		if(*it == '\n')
			++line_counter;

		++not_valid_ins;
	}
  }

  //analiza b³edów z liczników
  if(loop_call_stack.empty() == false) //jest coœ do œci¹gniêcia - brak odpowiadaj¹cego ]
  {	
	 while(loop_call_stack.empty() == false)
	 {
		errors->AddMessage(ParseErrors::ecLoopUnmatchedL, loop_call_stack.top());
		loop_call_stack.pop();
	 }
  }

  if(func_call_stack.empty() == false) //jest coœ do œci¹gniêcia - brak odpowiadaj¹cego )
  {	
	 while(func_call_stack.empty() == false)
	 {
		errors->AddMessage(ParseErrors::ecUnmatchedFunEnd, func_call_stack.top());
		func_call_stack.pop();
	 }
  }
  

  
}

void Parser::GetCode(CodeTape &c)
{
	c.Copy(precode.begin(), precode.end());
}

std::vector<CodeTape::bt_instruction> * Parser::GetCode()
{
	return &precode;
}

bool Parser::isCodeValid(void)
{
	return errors->ErrorsCount() == 0;
}

bool Parser::isValidOperator(char &c)
{
	static const char *valid_bt_ops = "<>+-.,[]()&{}!#$%/\\@;:";
	static const char *valid_bf_ops = "<>+-.,[]";
	static const char *valid_pb_ops = "<>+-.,[]():";
	static const char *valid_bo_ops = "<>+-.,[]Y";
	
	switch(language)
	{
		case CodeTape::clBrainThread: return strchr(valid_bt_ops,c) != 0; 
		case CodeTape::clBrainFuck:   return strchr(valid_bf_ops,c) != 0;
		case CodeTape::clPBrain:      return strchr(valid_pb_ops,c) != 0;
		case CodeTape::clBrainFork:   return strchr(valid_bo_ops,c) != 0;
		default:
			 return strchr(valid_bt_ops,c) != 0;
	}

	return false;
}

bool Parser::isValidDebugOperator(char &c)
{
	static const char *valid_db_ops = "MTFSD";
	
	if(language == CodeTape::clBrainFuck && c == '#')
		return true;

	return strchr(valid_db_ops,c) != 0;
}

CodeTape::bt_operation Parser::MapCharToOperator(char &c)
{
	switch(language)	
	{
		case CodeTape::clBrainThread:
		{
			switch(c)
			{
				case '<': return CodeTape::btoMoveLeft;
				case '>': return CodeTape::btoMoveRight;
				case '+': return CodeTape::btoIncrement;
				case '-': return CodeTape::btoDecrement;
				case '.': return CodeTape::btoAsciiWrite;
				case ',': return CodeTape::btoAsciiRead;
				case '[': return CodeTape::btoBeginLoop;
				case ']': return CodeTape::btoEndLoop;

			    case '{': return CodeTape::btoFork;
				case '}': return CodeTape::btoJoin;
				case '!': return CodeTape::btoTerminate;

				case '(': return CodeTape::btoBeginFunction;
				case ')': return CodeTape::btoEndFunction;
				case '&': return CodeTape::btoCallFunction;

				case '#': return CodeTape::btoPush;
				case '$': return CodeTape::btoPop;
				case '%': return CodeTape::btoSwap;
				case '/': return CodeTape::btoSharedPush;
				case '\\':return CodeTape::btoSharedPop;
				case '@': return CodeTape::btoSharedSwap;

				case ':': return CodeTape::btoDecimalWrite;
				case ';': return CodeTape::btoDecimalRead;
			}
		}
		break;
		case CodeTape::clBrainFuck:
		{
			switch(c)
			{
				case '<': return CodeTape::btoMoveLeft;
				case '>': return CodeTape::btoMoveRight;
				case '+': return CodeTape::btoIncrement;
				case '-': return CodeTape::btoDecrement;
				case '.': return CodeTape::btoAsciiWrite;
				case ',': return CodeTape::btoAsciiRead;
				case '[': return CodeTape::btoBeginLoop;
				case ']': return CodeTape::btoEndLoop;
			}
		}
		break;
		case CodeTape::clPBrain:
		{
			switch(c)
			{
				case '<': return CodeTape::btoMoveLeft;
				case '>': return CodeTape::btoMoveRight;
				case '+': return CodeTape::btoIncrement;
				case '-': return CodeTape::btoDecrement;
				case '.': return CodeTape::btoAsciiWrite;
				case ',': return CodeTape::btoAsciiRead;
				case '[': return CodeTape::btoBeginLoop;
				case ']': return CodeTape::btoEndLoop;
				case '(': return CodeTape::btoBeginFunction;
				case ')': return CodeTape::btoEndFunction;
				case ':': return CodeTape::btoCallFunction;
			}
		}
		break;
		case CodeTape::clBrainFork:
		{
			switch(c)
			{
				case '<': return CodeTape::btoMoveLeft;
				case '>': return CodeTape::btoMoveRight;
				case '+': return CodeTape::btoIncrement;
				case '-': return CodeTape::btoDecrement;
				case '.': return CodeTape::btoAsciiWrite;
				case ',': return CodeTape::btoAsciiRead;
				case '[': return CodeTape::btoBeginLoop;
				case ']': return CodeTape::btoEndLoop;
			    case 'Y': return CodeTape::btoFork;
			}
		}
		break;
	}

	if(debug_instructions_mode)
	{
		if(language == CodeTape::clBrainFuck)	
		{
			return CodeTape::btoDEBUG_SimpleMemoryDump;
		}
		else
		{
			switch(c)
			{
				case 'M': return CodeTape::btoDEBUG_SimpleMemoryDump;
				case 'D': return CodeTape::btoDEBUG_MemoryDump;
				case 'F': return CodeTape::btoDEBUG_FunctionsStackDump;
				case 'S': return CodeTape::btoDEBUG_StackDump;
				case 'T': return CodeTape::btoDEBUG_ThreadInfoDump;
			}
		}
	}

	return CodeTape::btoInvalid;
}

unsigned int inline Parser::GetValidPos(std::vector<char>::iterator &pos, std::vector<char>::iterator &begin, unsigned int not_valid_pos)
{
	//if(pos - begin - not_valid_pos) < 0
		return pos - begin - not_valid_pos;
}
