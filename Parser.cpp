#include "Parser.h"

#include <cstring>
#include <stack>
#include <algorithm>

/*
 * Klasa Parsera
 * Parser analizuje kod, odrzuca zbêdne znaki i przygotowuje go do interpretacji (np. ³aczy intrukcje 
 * pocz¹tku i koñca pêtli, aby szybiej dokonywaæ skoków). Mo¿e tez debugowaæ i optymalizowaæ kod
*/

Parser::Parser(MessageLog *messages, bool debug_instructions_on)
{
	language = CodeTape::clBrainThread;
	
	if(messages == nullptr)
		throw std::invalid_argument("Parser::Parser: MessageLog * = null");
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
	  errors->AddMessage(MessageLog::ecEmptyCode, 0);
	  return;
  }

  precode.reserve(source.size()/2);
  
  for(std::vector<char>::iterator it = source.begin(); it < source.end(); ++it)
  {
	if(isValidOperator(*it) || (debug_instructions_mode && isValidDebugOperator(*it)) )
	{
		curr_op = MapCharToOperator(*it);
		
		if(curr_op == CodeTape::btoBeginLoop /*|| curr_op == CodeTape::btoInvBeginLoop*/) //slepe wi¹zanie
		{
			loop_call_stack.push(GetValidPos(it, source.begin(), not_valid_ins));
			precode.push_back(CodeTape::bt_instruction(curr_op));
		}
		else if(curr_op == CodeTape::btoEndLoop /*|| curr_op == CodeTape::btoInvEndLoop*/)
		{
			if(loop_call_stack.empty() == false) 
			{
				precode[ loop_call_stack.top() ].jump = GetValidPos(it, source.begin(), not_valid_ins);
				precode.push_back(CodeTape::bt_instruction(curr_op, loop_call_stack.top()));
				
				//szukamy [ ( ] )
				if(func_call_stack.empty() == false && loop_call_stack.top() < func_call_stack.top() && func_call_stack.top() < GetValidPos(it, source.begin(), not_valid_ins)) 
				{
					errors->AddMessage(MessageLog::ecBLOutOfFunctionScope, GetValidPos(it, source.begin(), not_valid_ins), line_counter);
				}

				loop_call_stack.pop();
			}
			else //nie ma nic do œci¹gniêcia - brak odpowiadaj¹cego [
			{
				errors->AddMessage(MessageLog::ecUnmatchedLoopBegin, GetValidPos(it, source.begin(), not_valid_ins), line_counter);
			}
					
		}
		else if(curr_op == CodeTape::btoBeginFunction) 
		{
			func_call_stack.push(GetValidPos(it, source.begin(), not_valid_ins));
			precode.push_back(CodeTape::bt_instruction(curr_op));
		}
		else if(curr_op == CodeTape::btoEndFunction)
		{
			if(func_call_stack.empty() == false) 
			{
				precode[ func_call_stack.top() ].jump = GetValidPos(it, source.begin(), not_valid_ins);
				precode.push_back(CodeTape::bt_instruction(curr_op, func_call_stack.top()));

				//szukamy ( [ ) ]
				if(loop_call_stack.empty() == false && func_call_stack.top() < loop_call_stack.top() && loop_call_stack.top() < GetValidPos(it, source.begin(), not_valid_ins)) 
				{
					errors->AddMessage(MessageLog::ecELOutOfFunctionScope, GetValidPos(it, source.begin(), not_valid_ins), line_counter);
				}

				func_call_stack.pop();
			}
			else //nie ma nic do œci¹gniêcia - brak odpowiadaj¹cego (
			{
				errors->AddMessage(MessageLog::ecUnmatchedFunBegin, GetValidPos(it, source.begin(), not_valid_ins), line_counter);
			}
					
		}
		/*else if(curr_op == CodeTape::btoBreak)//break w brainlove
		{
			std::vector<char>::iterator br_it = std::find(it, source.end(), ']');
			if(it != source.end()) 
			{
				precode.push_back(CodeTape::bt_instruction(curr_op, loop_call_stack.top()));
			}
			else 
			{
				errors->AddMessage(MessageLog::ecUnmatchedBreak, GetValidPos(it, source.begin(), not_valid_ins), line_counter);
			}		
		}*/
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
		errors->AddMessage(MessageLog::ecUnmatchedLoopEnd, loop_call_stack.top());
		loop_call_stack.pop();
	 }
  }

  if(func_call_stack.empty() == false) //jest coœ do œci¹gniêcia - brak odpowiadaj¹cego )
  {	
	 while(func_call_stack.empty() == false)
	 {
		errors->AddMessage(MessageLog::ecUnmatchedFunEnd, func_call_stack.top());
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
		switch(language)	
		{
			case CodeTape::clBrainThread:
			{
				switch(c)
				{
					case 'M': return CodeTape::btoDEBUG_SimpleMemoryDump;
					case 'D': return CodeTape::btoDEBUG_MemoryDump;
					case 'F': return CodeTape::btoDEBUG_FunctionsStackDump;
					case 'E': return CodeTape::btoDEBUG_FunctionsDefsDump;
					case 'S': return CodeTape::btoDEBUG_StackDump;
					case 'H': return CodeTape::btoDEBUG_SharedStackDump;
					case 'T': return CodeTape::btoDEBUG_ThreadInfoDump;
				}
			}
			break;
			case CodeTape::clBrainFuck:
			{
				switch(c)
				{
					case '#':
					case 'M': return CodeTape::btoDEBUG_SimpleMemoryDump;
					case 'D': return CodeTape::btoDEBUG_MemoryDump;
				}
			}
			break;
			case CodeTape::clPBrain:
			{
				switch(c)
				{
					case '#':
				    case 'M': return CodeTape::btoDEBUG_SimpleMemoryDump;
					case 'D': return CodeTape::btoDEBUG_MemoryDump;
					case 'F': return CodeTape::btoDEBUG_FunctionsStackDump;
					case 'E': return CodeTape::btoDEBUG_FunctionsDefsDump;
				}
			}
			break;
			case CodeTape::clBrainFork:
			{
				switch(c)
				{
					case '#':
				    case 'M': return CodeTape::btoDEBUG_SimpleMemoryDump;
					case 'D': return CodeTape::btoDEBUG_MemoryDump;
					case 'T': return CodeTape::btoDEBUG_ThreadInfoDump;
				}
			}
			break;
		}

	}

	return CodeTape::btoInvalid;
}

unsigned int inline Parser::GetValidPos(std::vector<char>::iterator &pos, std::vector<char>::iterator &begin, unsigned int not_valid_pos)
{
	return pos - begin - not_valid_pos;
}
