#include "Parser.h"
#include "MessageLog.h"

#include <cstring>
#include <stack>
#include <algorithm>
#include <iterator>

/*
 * Klasa Parsera
 * Parser analizuje kod, odrzuca zbêdne znaki i przygotowuje go do interpretacji (np. ³aczy intrukcje 
 * pocz¹tku i koñca pêtli, aby szybiej dokonywaæ skoków). Mo¿e tez debugowaæ i optymalizowaæ kod
*/

Parser::Parser(CodeLang lang, bool debug_instructions_mode)
{
	language = lang;
	this->debug_instructions_mode = debug_instructions_mode;
}

void Parser::Parse(const char * data)
{
	std::vector<char> buffer (data, data + strlen(data));
	Parse(buffer);
}

void Parser::Parse(std::ifstream &in)
{	
	if(in.fail())
		throw std::ios_base::failure("File read error");

	//ladowanie z pliku
	std::vector<char> buffer;
	std::copy(std::istream_iterator<char>(in), std::istream_iterator<char>(), std::back_inserter(buffer));
	
	Parse(buffer);
}


void Parser::Parse(std::vector<char> &source)
{
  std::stack<unsigned int> loop_call_stack;
  std::stack<unsigned int> func_call_stack;

  //for loop jumps
  unsigned int not_valid_ins = 0;

  //current op
  CodeTape::bt_operation curr_op;

  //next stack op will be executed on shared stack
  bool switchToSharedHeap = false;

  if(source.empty())
  {
	  MessageLog::Instance().AddMessage(MessageLog::ecEmptyCode, 0);
	  return;
  }
  else if(this->language == Parser::clAuto)
  {
	  this->language = RecognizeLang(source);
  }

  instructions.reserve(source.size());
  
  for(std::vector<char>::iterator it = source.begin(); it < source.end(); ++it)
  {
	if(isValidOperator(*it) || (debug_instructions_mode && isValidDebugOperator(*it)) )
	{
		curr_op = MapCharToOperator(*it);
		
		if(curr_op == CodeTape::btoBeginLoop /*|| curr_op == CodeTape::btoInvBeginLoop*/) //slepe wi¹zanie
		{
			loop_call_stack.push(GetValidPos(it, source.begin(), not_valid_ins));
			instructions.push_back(CodeTape::bt_instruction(curr_op));
		}
		else if(curr_op == CodeTape::btoEndLoop /*|| curr_op == CodeTape::btoInvEndLoop*/)
		{
			if(loop_call_stack.empty() == false) 
			{
				instructions[ loop_call_stack.top() ].jump = GetValidPos(it, source.begin(), not_valid_ins);
				instructions.push_back(CodeTape::bt_instruction(curr_op, loop_call_stack.top()));
				
				//szukamy [ ( ] )
				if(func_call_stack.empty() == false && loop_call_stack.top() < func_call_stack.top() && func_call_stack.top() < GetValidPos(it, source.begin(), not_valid_ins)) 
				{
					MessageLog::Instance().AddMessage(MessageLog::ecBLOutOfFunctionScope, GetValidPos(it, source.begin(), not_valid_ins));
				}

				loop_call_stack.pop();
			}
			else //nie ma nic do œci¹gniêcia - brak odpowiadaj¹cego [
			{
				MessageLog::Instance().AddMessage(MessageLog::ecUnmatchedLoopBegin, GetValidPos(it, source.begin(), not_valid_ins));
			}
					
		}
		else if(curr_op == CodeTape::btoBeginFunction) 
		{
			func_call_stack.push(GetValidPos(it, source.begin(), not_valid_ins));
			instructions.push_back(CodeTape::bt_instruction(curr_op));
		}
		else if(curr_op == CodeTape::btoEndFunction)
		{
			if(func_call_stack.empty() == false) 
			{
				instructions[ func_call_stack.top() ].jump = GetValidPos(it, source.begin(), not_valid_ins);
				instructions.push_back(CodeTape::bt_instruction(curr_op, func_call_stack.top()));

				//szukamy ( [ ) ]
				if(loop_call_stack.empty() == false && func_call_stack.top() < loop_call_stack.top() && loop_call_stack.top() < GetValidPos(it, source.begin(), not_valid_ins)) 
				{
					MessageLog::Instance().AddMessage(MessageLog::ecELOutOfFunctionScope, GetValidPos(it, source.begin(), not_valid_ins));
				}

				func_call_stack.pop();
			}
			else //nie ma nic do œci¹gniêcia - brak odpowiadaj¹cego (
			{
				MessageLog::Instance().AddMessage(MessageLog::ecUnmatchedFunBegin, GetValidPos(it, source.begin(), not_valid_ins));
			}
					
		}
		/*else if(curr_op == CodeTape::btoBreak)//break w brainlove
		{
			std::vector<char>::iterator br_it = std::find(it, source.end(), ']');
			if(it != source.end()) 
			{
				instructions.push_back(CodeTape::bt_instruction(curr_op, loop_call_stack.top()));
			}
			else 
			{
				errors->AddMessage(MessageLog::ecUnmatchedBreak, GetValidPos(it, source.begin(), not_valid_ins), line_counter);
			}		
		}*/
		else if(switchToSharedHeap && 
			(curr_op == CodeTape::btoPush || curr_op == CodeTape::btoPop || curr_op == CodeTape::btoSwap))
		{
			switchToSharedHeap = false;

			switch(curr_op)
			{
				case CodeTape::btoPush: instructions.push_back(CodeTape::bt_instruction(CodeTape::btoSharedPush)); 
					break;
				case CodeTape::btoPop: instructions.push_back(CodeTape::bt_instruction(CodeTape::btoSharedPop)); 
					break;
				case CodeTape::btoSwap: instructions.push_back(CodeTape::bt_instruction(CodeTape::btoSharedSwap)); 
					break;
			}
		}
		else if(curr_op == CodeTape::btoSwitchHeap) 
		{
			switchToSharedHeap = true; //non executable operation
			instructions.push_back(CodeTape::bt_instruction(curr_op));
		}
		else 
			instructions.push_back(CodeTape::bt_instruction(curr_op));
	}
	else
	{	
		++not_valid_ins;
	}
  }

  //analiza b³edów z liczników
  if(loop_call_stack.empty() == false) //jest coœ do œci¹gniêcia - brak odpowiadaj¹cego ]
  {	
	 while(loop_call_stack.empty() == false)
	 {
		MessageLog::Instance().AddMessage(MessageLog::ecUnmatchedLoopEnd, loop_call_stack.top());
		loop_call_stack.pop();
	 }
  }

  if(func_call_stack.empty() == false) //jest coœ do œci¹gniêcia - brak odpowiadaj¹cego )
  {	
	 while(func_call_stack.empty() == false)
	 {
		MessageLog::Instance().AddMessage(MessageLog::ecUnmatchedFunEnd, func_call_stack.top());
		func_call_stack.pop();
	 }
  }
  //koniec
}

std::vector<CodeTape::bt_instruction> * Parser::GetCode()
{
	return &instructions;
}

bool Parser::isCodeValid(void)
{
	return MessageLog::Instance().ErrorsCount() == 0;
}

bool Parser::isValidOperator(char &c)
{
	static const char *valid_bt_ops = "<>+-.,[]()*{}!#^%~;:";
	static const char *valid_bf_ops = "<>+-.,[]";
	static const char *valid_pb_ops = "<>+-.,[]():";
	static const char *valid_bo_ops = "<>+-.,[]Y";
	
	switch(language)
	{
		case Parser::clBrainThread: return strchr(valid_bt_ops,c) != 0; 
		case Parser::clBrainFuck:   return strchr(valid_bf_ops,c) != 0;
		case Parser::clPBrain:      return strchr(valid_pb_ops,c) != 0;
		case Parser::clBrainFork:   return strchr(valid_bo_ops,c) != 0;
		default:
			 return strchr(valid_bt_ops,c) != 0;
	}

	return false;
}

bool Parser::isValidDebugOperator(char &c)
{
	static const char *valid_db_ops = "MTFSD";
	
	if(language == Parser::clBrainFuck && c == '#')
		return true;

	return strchr(valid_db_ops,c) != 0;
}

CodeTape::bt_operation Parser::MapCharToOperator(char &c)
{
	switch(language)	
	{
		case Parser::clBrainThread:
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
				case '*': return CodeTape::btoCallFunction;

				case '#': return CodeTape::btoPush;
				case '^': return CodeTape::btoPop;
				case '%': return CodeTape::btoSwap;
				case '~': return CodeTape::btoSwitchHeap;
				//case '/': return CodeTape::btoSharedPush;
				//case '\\':return CodeTape::btoSharedPop;
				//case '@': return CodeTape::btoSharedSwap;

				case ':': return CodeTape::btoDecimalWrite;
				case ';': return CodeTape::btoDecimalRead;
			}
		}
		break;
		case Parser::clBrainFuck:
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
		case Parser::clPBrain:
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
		case Parser::clBrainFork:
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
			case Parser::clBrainThread:
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
			case Parser::clBrainFuck:
			{
				switch(c)
				{
					case '#':
					case 'M': return CodeTape::btoDEBUG_SimpleMemoryDump;
					case 'D': return CodeTape::btoDEBUG_MemoryDump;
				}
			}
			break;
			case Parser::clPBrain:
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
			case Parser::clBrainFork:
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

Parser::CodeLang Parser::RecognizeLang(std::vector<char> &source)
{
	std::vector<char>::iterator a, b;

	a = std::find_if(source.begin(), source.end(), 
				             [](const char &c){ return c == '('; });

	b = std::find_if(source.begin(), source.end(), 
				             [](const char &c){ return c == ')'; });

	if(a < b && b != source.end()) //uwaga, mamy funkcje jakby [bt, pb]
	{
		a = std::find_if(source.begin(), source.end(), 
				             [](const char &c){ return c == '*'; });

		b = std::find_if(source.begin(), source.end(), 
				             [](const char &c){ return c == ':'; });


		if(a == source.end() && b != source.end()) //mamy call w stylu pb, a nie mamy w stylu bt
		{
			return Parser::clPBrain;
		}
		else if(a != source.end() && b != source.end()) //mamy call w stylu bt, i decimal output
		{
			return Parser::clBrainThread;
		}

	}
	else //bf, b
	{
		a = std::find_if(source.begin(), source.end(), 
				             [](const char &c){ return c == 'Y'; });

		if(a != source.end() && a+1 != source.end() && *(a+1) == '[') //przy rozwidleniu powinno stac otwarcie petli, inaczej to moze byc komentarz
		{
			return Parser::clBrainFork;
		}
	}

	return Parser::clBrainFuck;
}

unsigned int inline Parser::GetValidPos(std::vector<char>::iterator &pos, std::vector<char>::iterator &begin, unsigned int &not_valid_pos)
{
	return pos - begin - not_valid_pos;
}

