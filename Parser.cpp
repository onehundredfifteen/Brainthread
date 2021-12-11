#include "Parser.h"
#include "MessageLog.h"
#include "CodeOptimizer.h"

#include <cstring>
#include <stack>
#include <algorithm>
#include <iterator>

using namespace CodeTape;

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

  //optimizer
  std::list<unsigned int> optimizer_entrypoint;
  bool _optimize = true;

  //for loop jumps
  unsigned int ignore_ins = 0;

  //current op
  CodeTape::bt_operation curr_op;
  std::vector<char>::iterator _it;
  int reps = 0;

  //next stack op will be executed on shared stack
  bool switchToSharedHeap = false;
  unsigned int switchJump = 0;

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
		
		if(curr_op == bt_operation::btoBeginLoop /*|| curr_op == CodeTape::btoInvBeginLoop*/) //slepe wi¹zanie
		{
			
			if (_optimize && (optimizer_entrypoint.empty() ||
				instructions[optimizer_entrypoint.back()].operation != bt_operation::btoBeginLoop)) {
				
				_it = (it + 2);
				if(_it < source.end() && 
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
			else {
				loop_call_stack.push(GetValidPos(it, source.begin(), ignore_ins));
				instructions.push_back(bt_instruction(curr_op));
			}
		}
		else if(curr_op == bt_operation::btoEndLoop /*|| curr_op == CodeTape::btoInvEndLoop*/)
		{
			if(loop_call_stack.empty() == false) 
			{
				instructions[ loop_call_stack.top() ].jump = GetValidPos(it, source.begin(), ignore_ins);
				instructions.push_back(bt_instruction(curr_op, loop_call_stack.top()));
				
				//szukamy [ ( ] )
				if(func_call_stack.empty() == false && loop_call_stack.top() < func_call_stack.top() && func_call_stack.top() < GetValidPos(it, source.begin(), ignore_ins)) 
				{
					MessageLog::Instance().AddMessage(MessageLog::ecBLOutOfFunctionScope, GetValidPos(it, source.begin(), ignore_ins));
				}

				loop_call_stack.pop();
			}
			else //nie ma nic do œci¹gniêcia - brak odpowiadaj¹cego [
			{
				MessageLog::Instance().AddMessage(MessageLog::ecUnmatchedLoopBegin, GetValidPos(it, source.begin(), ignore_ins));
			}
					
		}
		else if(curr_op == bt_operation::btoBeginFunction) 
		{
			func_call_stack.push(GetValidPos(it, source.begin(), ignore_ins));
			instructions.push_back(CodeTape::bt_instruction(curr_op));
		}
		else if(curr_op == bt_operation::btoEndFunction)
		{
			if(func_call_stack.empty() == false) 
			{
				instructions[ func_call_stack.top() ].jump = GetValidPos(it, source.begin(), ignore_ins);
				instructions.push_back(CodeTape::bt_instruction(curr_op, func_call_stack.top()));

				//szukamy ( [ ) ]
				if(loop_call_stack.empty() == false && func_call_stack.top() < loop_call_stack.top() && loop_call_stack.top() < GetValidPos(it, source.begin(), ignore_ins)) 
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
		/*else if(curr_op == CodeTape::btoBreak)//break w brainlove
		{
			std::vector<char>::iterator br_it = std::find(it, source.end(), ']');
			if(it != source.end()) 
			{
				instructions.push_back(CodeTape::bt_instruction(curr_op, loop_call_stack.top()));
			}
			else 
			{
				errors->AddMessage(MessageLog::ecUnmatchedBreak, GetValidPos(it, source.begin(), ignore_ins), line_counter);
			}		
		}*/
		else if(switchToSharedHeap && 
			(curr_op == bt_operation::btoPush || curr_op == bt_operation::btoPop || curr_op == bt_operation::btoSwap))
		{
			switchToSharedHeap = false;

			switch(curr_op)
			{
				case bt_operation::btoPush: instructions.push_back(bt_instruction(bt_operation::btoSharedPush)); 
					break;
				case bt_operation::btoPop: instructions.push_back(CodeTape::bt_instruction(bt_operation::btoSharedPop)); 
					break;
				case bt_operation::btoSwap: instructions.push_back(CodeTape::bt_instruction(bt_operation::btoSharedSwap)); 
					break;
			}

			instructions.back().jump = switchJump;
		}
		else if(curr_op == bt_operation::btoSwitchHeap) 
		{
			switchToSharedHeap = true; //non executable operation
			switchJump = GetValidPos(it, source.begin(), ignore_ins);
			//instructions.push_back(bt_operation::bt_instruction(curr_op));
		}
		else if(_optimize && CodeOptimizer::isOptimizable(curr_op)) {
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

			instructions.push_back(CodeTape::bt_instruction(curr_op, UINT_MAX, reps));
		}
		else 
			instructions.push_back(CodeTape::bt_instruction(curr_op));
	}
	else
	{	
		++ignore_ins;
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

  if (0&&_optimize) {
	  //CodeOptimizer optimizer(optimizer_entrypoint, &instructions, coLevel::co1);
	  //optimizer.Optimize();
  }
  //koniec
}

CodeTape::Tape Parser::GetCode()
{
	return std::move(instructions);
}
 
bool Parser::isCodeValid(void) const
{
	return MessageLog::Instance().ErrorsCount() == 0;
}

bool Parser::isValidOperator(const char &c) const
{
	static const char *valid_bt_ops = "<>+-.,[]()*{}!&^%~;:";
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

bool Parser::isValidDebugOperator(const char &c) const
{
	static const char *valid_db_ops = "MTFSD";
	
	if(language == Parser::clBrainFuck && c == '#')
		return true;

	return strchr(valid_db_ops,c) != 0;
}

CodeTape::bt_operation Parser::MapCharToOperator(const char &c) const
{
	switch(language)	
	{
		case Parser::clBrainThread:
		{
			switch(c)
			{
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
		break;
		case Parser::clBrainFuck:
		{
			switch(c)
			{
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
		break;
		case Parser::clPBrain:
		{
			switch(c)
			{
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
		break;
		case Parser::clBrainFork:
		{
			switch(c)
			{
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
					case 'M': return bt_operation::btoDEBUG_SimpleMemoryDump;
					case 'D': return bt_operation::btoDEBUG_MemoryDump;
					case 'F': return bt_operation::btoDEBUG_FunctionsStackDump;
					case 'E': return bt_operation::btoDEBUG_FunctionsDefsDump;
					case 'S': return bt_operation::btoDEBUG_StackDump;
					case 'H': return bt_operation::btoDEBUG_SharedStackDump;
					case 'T': return bt_operation::btoDEBUG_ThreadInfoDump;
				}
			}
			break;
			case Parser::clBrainFuck:
			{
				switch(c)
				{
					case '#':
					case 'M': return bt_operation::btoDEBUG_SimpleMemoryDump;
					case 'D': return bt_operation::btoDEBUG_MemoryDump;
				}
			}
			break;
			case Parser::clPBrain:
			{
				switch(c)
				{
					case '#':
				    case 'M': return bt_operation::btoDEBUG_SimpleMemoryDump;
					case 'D': return bt_operation::btoDEBUG_MemoryDump;
					case 'F': return bt_operation::btoDEBUG_FunctionsStackDump;
					case 'E': return bt_operation::btoDEBUG_FunctionsDefsDump;
				}
			}
			break;
			case Parser::clBrainFork:
			{
				switch(c)
				{
					case '#':
				    case 'M': return bt_operation::btoDEBUG_SimpleMemoryDump;
					case 'D': return bt_operation::btoDEBUG_MemoryDump;
					case 'T': return bt_operation::btoDEBUG_ThreadInfoDump;
				}
			}
			break;
		}

	}

	return bt_operation::btoInvalid;
}

Parser::CodeLang Parser::RecognizeLang(std::vector<char> &source) const
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
	else //haven't functions
	{
		a = std::find_if(source.begin(), source.end(), 
				             [](const char &c){ return c == 'Y'; });

		if(a != source.end() && a+1 != source.end() && *(a+1) == '[') //przy rozwidleniu powinno stac otwarcie petli, inaczej to moze byc komentarz
		{
			return Parser::clBrainFork;
		}

		//next guess
		a = std::find_if(source.begin(), source.end(), 
							 [](const char &c){ return strchr("}~^%&", c) != 0; });

		if(a != source.end()) //przy rozwidleniu powinno stac otwarcie petli, inaczej to moze byc komentarz
		{
			return Parser::clBrainThread;
		}

		a = std::find_if(source.begin(), source.end(), 
				             [](const char &c){ return c == '{'; });

		if(a != source.end() && a+1 != source.end() && *(a+1) == '[') //przy rozwidleniu powinno stac otwarcie petli, inaczej to moze byc komentarz
		{
			return Parser::clBrainThread;
		}
	}

	return Parser::clBrainFuck;
}

unsigned int inline Parser::GetValidPos(const std::vector<char>::iterator &pos, const std::vector<char>::iterator &begin, unsigned int &not_valid_pos) const
{
	return pos - begin - not_valid_pos;
}

