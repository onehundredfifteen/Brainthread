#include "Parser.h"
#include "BrainThreadRuntimeException.h"

#include <cstring>
#include <stack>

#include <iostream>

Parser::Parser(void)
{
	language = clBrainThread;
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



  precode.reserve(source.size());
  
  for(std::vector<char>::iterator it = source.begin(); it < source.end(); ++it)
  {
	if(isValidOperator(*it))
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
				errors.AddMessage(ParseErrors::ecLoopUnmatchedR, GetValidPos(it, source.begin(), not_valid_ins), line_counter);
			}
					
		}
		else if(curr_op == CodeTape::btoBeginFunction && (language == clBrainThread || language == clPBrain)) 
		{
			func_call_stack.push(GetValidPos(it, source.begin(), not_valid_ins));
			precode.push_back(CodeTape::bt_instruction(curr_op));
		}
		else if(curr_op == CodeTape::btoEndFunction && (language == clBrainThread || language == clPBrain))
		{
			if(func_call_stack.empty() == false) 
			{
				precode[ func_call_stack.top() ].jump = GetValidPos(it, source.begin(), not_valid_ins);
				precode.push_back(CodeTape::bt_instruction(curr_op, func_call_stack.top()));
				loop_call_stack.pop();
			}
			else //nie ma nic do œci¹gniêcia - brak odpowiadaj¹cego (
			{
				errors.AddMessage(ParseErrors::ecUnmatchedFunBegin, GetValidPos(it, source.begin(), not_valid_ins), line_counter);
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
		errors.AddMessage(ParseErrors::ecLoopUnmatchedL, loop_call_stack.top());
		loop_call_stack.pop();
	 }
  }

  if(func_call_stack.empty() == false) //jest coœ do œci¹gniêcia - brak odpowiadaj¹cego )
  {	
	 while(func_call_stack.empty() == false)
	 {
		errors.AddMessage(ParseErrors::ecUnmatchedFunEnd, func_call_stack.top());
		func_call_stack.pop();
	 }
  }
  

  
}

void Parser::GetCode(CodeTape &c)
{
	c.Copy(precode.begin(), precode.end());
}


bool Parser::isCodeValid(void)
{
	return errors.ErrorsCount() == 0;
}
void Parser::GetMessages(void)
{
	errors.GetMessages();
}
unsigned Parser::MessageCount(void)
{
	return errors.ErrorsCount() + errors.WarningsCount();
}
		 
			


//ponizsze funkcje szukaj¹ prawej instrukcji "], ) lub }" pocz¹wszy od from_pos.
unsigned int Parser::FindMatchingRightPair(CodeTape::bt_operation op, unsigned int from_pos) 
{
  /* int lcnt = 1;
   unsigned int cp = from_pos+1;
   CodeTape::bt_operation to_find, z; 

   if(op != CodeTape::btoBeginLoop || op != CodeTape::btoBeginFunction || op != CodeTape::btoFork)
   {
	   //throw wyj¹tek
   }

   to_find = (op == CodeTape::btoBeginLoop ? CodeTape::btoEndLoop : (op == CodeTape::btoBeginFunction ? CodeTape::btoEndFunction : CodeTape::btoWait));
   //pêtle do try catcgh bo wyjdzie i posze wyjatek
   while(true)
   {
	 z = instructions[ cp ].operation;

	 if( z == op) ++lcnt;
	 else if( z == to_find)  --lcnt;

	 if(lcnt == 0)
		 return cp;

	 ++cp;
   }*/
	return 0;
}

//ponizsze funkcje szukaj¹ lewej instrukcji "[, ( lub {" pocz¹wszy od from_pos.
unsigned int Parser::FindMatchingLeftPair(CodeTape::bt_operation op, unsigned int from_pos) 
{
   /*int lcnt = 1;
   unsigned int cp = from_pos-1;
   CodeTape::bt_operation to_find, z; 

   if(op != CodeTape::btoEndLoop || op != CodeTape::btoEndFunction || op != CodeTape::btoWait)
   {
	   //throw wyj¹tek
   }

   to_find = (op == CodeTape::btoEndLoop ? CodeTape::btoBeginLoop : (op == CodeTape::btoEndFunction ? CodeTape::btoBeginFunction : CodeTape::btoFork));

   while(true)
   {
	 z = instructions[ cp ].operation;

	 if( z == op) ++lcnt;
	 else if( z == to_find)  --lcnt;

	 if(lcnt == 0)
		 return cp;

	 --cp;
   }*/
	return 0;
}

bool Parser::isValidOperator(char &c)
{
	static const char *valid_bt_ops = "<>+-.,[]()#{}'\"%/\\@:;";
	static const char *valid_bf_ops = "<>+-.,[]";
	static const char *valid_pb_ops = "<>+-.,[]():";
	static const char *valid_bo_ops = "<>+-.,[]Yy";
	
	switch(language)
	{
		case clBrainThread: return strchr(valid_bt_ops,c) != 0; 
		case clBrainFuck:   return strchr(valid_bf_ops,c) != 0;
		case clPBrain:      return strchr(valid_pb_ops,c) != 0;
		case clBrainFork:   return strchr(valid_bo_ops,c) != 0;
		default:
			 return strchr(valid_bt_ops,c) != 0;
	}

	return false;
}

CodeTape::bt_operation Parser::MapCharToOperator(char &c)
{
	switch(language)	
	{
		case clBrainThread:
		{
			switch(c)
			{
				case '<': return CodeTape::btoMoveLeft;
				case '>': return CodeTape::btoMoveRight;
				case '+': return CodeTape::btoIncrement;
				case '-': return CodeTape::btoDecrement;
				case '.': return CodeTape::btoStdWrite;
				case ',': return CodeTape::btoStdRead;
				case '[': return CodeTape::btoBeginLoop;
				case ']': return CodeTape::btoEndLoop;
			    case '{': return CodeTape::btoFork;
				case '}': return CodeTape::btoWait;
			}
		}
		break;
		case clBrainFuck:
		{
			switch(c)
			{
				case '<': return CodeTape::btoMoveLeft;
				case '>': return CodeTape::btoMoveRight;
				case '+': return CodeTape::btoIncrement;
				case '-': return CodeTape::btoDecrement;
				case '.': return CodeTape::btoStdWrite;
				case ',': return CodeTape::btoStdRead;
				case '[': return CodeTape::btoBeginLoop;
				case ']': return CodeTape::btoEndLoop;
			}
		}
		break;
		case clBrainFork:
		{
			switch(c)
			{
				case '<': return CodeTape::btoMoveLeft;
				case '>': return CodeTape::btoMoveRight;
				case '+': return CodeTape::btoIncrement;
				case '-': return CodeTape::btoDecrement;
				case '.': return CodeTape::btoStdWrite;
				case ',': return CodeTape::btoStdRead;
				case '[': return CodeTape::btoBeginLoop;
				case ']': return CodeTape::btoEndLoop;
			    case 'Y':
				case 'y': return CodeTape::btoFork;
			}
		}
		break;
	}
	return CodeTape::btoUnkown;
}

unsigned int inline Parser::GetValidPos(std::vector<char>::iterator &pos, std::vector<char>::iterator &begin, unsigned int not_valid_pos)
{
	
	//if(pos - begin - not_valid_pos) < 0
		return pos - begin - not_valid_pos;
}
