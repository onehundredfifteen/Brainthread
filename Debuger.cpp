#include "Debuger.h"

#include <algorithm>
#include <iostream>

Debuger::Debuger(ParseErrors *messages, std::vector<CodeTape::bt_instruction> *precode, bool repair)
{
	language = CodeTape::clBrainThread;
	
	if(messages == nullptr)
		this->messages = new ParseErrors;
	else
		this->messages = messages;

	this->precode = precode;
	this->repair = repair;

	repaired_issues = 0;

	function_limit = 3;
}

Debuger::~Debuger(void)
{
}

bool Debuger::IsChangingInstruction(const CodeTape::bt_instruction &op)
{
	return (op.operation == CodeTape::btoAsciiWrite ||
		   op.operation == CodeTape::btoDecimalWrite ||
		   op.operation == CodeTape::btoFork ||
		   op.operation == CodeTape::btoJoin ||
		   op.operation == CodeTape::btoPush ||
		   op.operation == CodeTape::btoSharedPush ||
		   op.operation == CodeTape::btoTerminate ||
		   
		   op.operation == CodeTape::btoDEBUG_FunctionsStackDump ||
		   op.operation == CodeTape::btoDEBUG_MemoryDump ||
		   op.operation == CodeTape::btoDEBUG_StackDump ||
		   op.operation == CodeTape::btoDEBUG_ThreadInfoDump ||
		   op.operation == CodeTape::btoDEBUG_SimpleMemoryDump ||
		   op.operation == CodeTape::btoInvalid ||
		   op.operation == CodeTape::btoUnkown) == false; 
}

bool Debuger::IsArithmeticInstruction(const CodeTape::bt_instruction &op)
{
	return (op.operation == CodeTape::btoIncrement ||
		   op.operation == CodeTape::btoDecrement); 
}

bool Debuger::IsArithmeticSafeInstruction(const CodeTape::bt_instruction &op)
{
	return (IsArithmeticInstruction(op) ||
		    IsChangingInstruction(op) == false) &&
		    (op.operation != CodeTape::btoAsciiWrite && op.operation != CodeTape::btoDecimalWrite);
}

bool Debuger::IsMoveInstruction(const CodeTape::bt_instruction &op)
{
	return (op.operation == CodeTape::btoMoveLeft ||
		   op.operation == CodeTape::btoMoveRight); 
}

bool Debuger::IsMoveSafeInstruction(const CodeTape::bt_instruction &op)
{
     return (IsMoveInstruction(op) ||
		    IsChangingInstruction(op) == false) &&
		    (op.operation != CodeTape::btoAsciiWrite && op.operation != CodeTape::btoDecimalWrite &&
			 op.operation != CodeTape::btoSharedPush && op.operation == CodeTape::btoPush);
}


void Debuger::Debug(void)
{
	unsigned int c_size;
	
	function_def = 0;
	function_calls = 0;
	forks = 0;
	joins = 0;
	ignore_arithmetic_for_error = false;
	ignore_moves_for_error = false;
	
	for(std::vector<CodeTape::bt_instruction>::iterator it = precode->begin(); it < precode->end(); )
	{
		c_size = precode->size();
		//std::cout << "xxx " << precode->size() << " ccc ";
		TestForInfiniteLoops(it);
		if(c_size != precode->size())
			continue;

		TestForLoopsOutOfScope(it);
		if(c_size != precode->size())
			continue;

		TestForFunctionsErrors(it);
		if(c_size != precode->size())
			continue;

		TestForJoinBeforeFork(it);
		if(c_size != precode->size())
			continue;

		TestArithmeticsLoops(it);
		if(c_size != precode->size())
			continue;

		if(ignore_arithmetic_for_error == false)
		{
			TestArithmetics(it);
			if(c_size != precode->size())
				continue;
		}

		if(ignore_moves_for_error == false)
		{
			TestRedundantMoves(it);
			if(c_size != precode->size())
				continue;
		}

		if(it->operation == CodeTape::btoFork) ++forks;
		else if(it->operation == CodeTape::btoJoin) ++joins;
		else if(it->operation == CodeTape::btoCallFunction) ++function_calls;

		if(TestForRepetition(it, CodeTape::btoJoin))
			messages->AddMessage(ParseErrors::ecJoinRepeat, it - precode->begin());

		if(TestForRepetition(it, CodeTape::btoTerminate))
			messages->AddMessage(ParseErrors::ecTerminateRepeat, it - precode->begin());

		
		if(IsArithmeticSafeInstruction(*it) == false && ignore_arithmetic_for_error == true) 
			ignore_arithmetic_for_error = false;

		if(IsMoveInstruction(*it) == false && ignore_moves_for_error == true) 
			ignore_moves_for_error = false;

	    ++it;
	}

	if( forks == 0 && joins > 0)
	{
		messages->AddMessage(ParseErrors::ecJoinButNoFork, 1);
	}

	if( function_def > function_limit)
	{
		messages->AddMessage(ParseErrors::ecFunctionLimitExceed, 1);
	}

	if( function_def > 0 && function_calls == 0)
	{
		messages->AddMessage(ParseErrors::ecFunctionExistsButNoCall, 1);
	}

	std::cout << std::endl;
	for(std::vector<CodeTape::bt_instruction>::iterator it = precode->begin(); it < precode->end(); ++it)
	{
		std::cout << it->operation << ' ';
	}
	std::cout << std::endl;
}

void Debuger::TestForInfiniteLoops(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	if(it->operation == CodeTape::btoBeginLoop)
	{
		//pusta pêtla []
		if( (it + 1)->operation == CodeTape::btoEndLoop)
		{ 
			messages->AddMessage(ParseErrors::ecInfiniteLoop, it - precode->begin());	
			if(repair)
			{
				it = precode->erase(it, it+2);
				++repaired_issues;
				return;
			}
		}
		//pusta pêtla [[x[x]xx]]
		else if( (it + 1)->operation ==  CodeTape::btoBeginLoop && (precode->begin() + (it->jump) - 1)->operation == CodeTape::btoEndLoop)
		{
			messages->AddMessage(ParseErrors::ecEmptyLoop, it - precode->begin());
			if(repair)
			{
				precode->erase(precode->begin() + (it->jump));
				it = precode->erase(it);
				++repaired_issues;
				return;
			}
		}	 
	}
}

void Debuger::TestForLoopsOutOfScope(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	std::vector<CodeTape::bt_instruction>::iterator n, l;

	if(it->operation == CodeTape::btoBeginLoop)
	{
		l = precode->begin() + it->jump;
		n = std::find_if(it, l,[](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoEndFunction; });
			
		if(n != l) //koniec pêtli poza funkcj¹ ( [ ) ]
		{
			messages->AddMessage(ParseErrors::ecELOutOfFunctionScope, it - precode->begin());	
			if(repair) //usuwamy pêtlê
			{
				precode->erase(l);
				it = precode->erase(it);
				++repaired_issues;
				return;
			}
		}
			
		n = std::find_if(it, l,[](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoBeginFunction; });
			
		if(n != l) //pocz¹tek pêtli poza funkcj¹ [ ( ] )
		{
			messages->AddMessage(ParseErrors::ecELOutOfFunctionScope, it - precode->begin());	
			if(repair) //usuwamy pêtlê
			{
				precode->erase(l);
				it = precode->erase(it);
				++repaired_issues;
				return;
			}
		} 
	}
}

void Debuger::TestForFunctionsErrors(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	std::vector<CodeTape::bt_instruction>::iterator n, m;

	if(it->operation == CodeTape::btoEndFunction)
	{
		//szukam (
		n = std::find_if(it, precode->end(),[](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoBeginFunction; });
			
		if(n != precode->end()) //jest - to sprawdzamy instrukcje pomiedzy ) i (
		{	
			if(n != std::find_if_not(it, n, IsChangingInstruction) || it+1 == n) //jest podejrzenie redefinicji, bo nie ma instrukcji zmieniaj¹cych wartoœæ miêdzy funkcjami
			{
				messages->AddMessage(ParseErrors::ecFunctionRedefinition, it - precode->begin());
			}
		}
	}
	else if(it->operation == CodeTape::btoBeginFunction) // przypadek ((x)x) + pusta funkcja
	{
		n = precode->begin() + it->jump;//bierzemy ca³¹ treœæ funkcji

		if(n - it == 1) //funkcja jest pusta? ()
		{
			messages->AddMessage(ParseErrors::ecEmptyFunction, it - precode->begin());	
			if(repair) //usuwamy funkcjê
			{
				precode->erase(n);
				it = precode->erase(it);
				++repaired_issues;
				return;
			}
		}
		else // ok ma coœ -> szukamy funkcji wewêtrznej (bez pierwszego elementu, bo to wlasnie '(' )
		{
			m = std::find_if(it+1, n,[](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoBeginFunction; });
			if(m != n) //mamy funkcjê wewnêtrzn¹
			{
				++function_limit;
				if( m != std::find_if_not(it, m, IsChangingInstruction) || it+1 == m) //jest podejrzenie redefinicji, bo nie ma instrukcji zmieniaj¹cych wartoœæ miêdzy funkcjami (xx(
				{
					messages->AddMessage(ParseErrors::ecFunctionRedefinition2, it + 1 - precode->begin());
				}
			}
		}
		
		++function_def;	
		
	}
}

bool Debuger::TestForRepetition(std::vector<CodeTape::bt_instruction>::iterator &it, const CodeTape::bt_operation &op)
{
	std::vector<CodeTape::bt_instruction>::iterator n;

	if(it->operation == op && it+1 < precode->end() && (it+1)->operation == op)
	{
		n = std::find_if_not(it+1, precode->end(),[&op](const CodeTape::bt_instruction &o){ return o.operation == op; });
		if(repair) //usuwamy wszsytkie bez pierwszego
		{
			it = precode->erase(it+1, n);
			++repaired_issues;
		}
		else it = n;
	}
	else return false;

	return true;
}

void Debuger::TestForJoinBeforeFork(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	std::vector<CodeTape::bt_instruction>::iterator n, m;

	if(it->operation == CodeTape::btoJoin) // przypadki gdy join nie jest w funkcji
	{
        m = std::find_if(precode->begin(), it,[](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoBeginFunction; });

		if(m == it && forks == 0) //join poza funkcj¹ [mo¿e byc call do póŸniejszej funkcji z fork, ale to juz za trudne]
		{
			messages->AddMessage(ParseErrors::ecJoinBeforeFork, it - precode->begin());	
			if(repair) //usuwamy join
			{
				it = precode->erase(it);
				++repaired_issues;
			}
		}
		
	}
}

void Debuger::TestArithmetics(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	std::vector<CodeTape::bt_instruction>::iterator n;
	if(IsArithmeticInstruction(*it))
	{
		ignore_arithmetic_for_error = true;
		n = std::find_if_not(it, precode->end(),IsArithmeticSafeInstruction);
		//mamy ci¹g + - i instrukcji pomijalnych
		//std::cout <<"fakty " <<(n - it) <<"fakty " <<abs(Calcule(it,n)) <<"fakty " << count_if(it, n, IsArithmeticInstruction) <<"fakty " << Calcule(it,n)<<"$\n ";
		//instrukcji musi byc wiêcej niz jedna i wynik ma byc sosi¹gniêty najmniejsza liczba instrukcji, czyli bez np suma = 2 dla ++ [ops=2] a nie +-+-++ [ops=5]
		if((n - it) > 1 && abs(Calcule(it,n)) != std::count_if(it, n, IsArithmeticInstruction))
		{
			messages->AddMessage(ParseErrors::ecRedundantArithmetic, it - precode->begin());	
			if(repair)
			{
				//it = precode->erase(it, it+2);
				//++repaired_issues;
				//return;
			}
		}
	}
}

void Debuger::TestArithmeticsLoops(std::vector<CodeTape::bt_instruction>::iterator &it)
{   
	std::vector<CodeTape::bt_instruction>::iterator a, b, x;
	int sum = 0, ops = 0;

	if(it->operation == CodeTape::btoBeginLoop)
	{
		//po teœcie na pust¹ petle mamy zagwarantowane, ze jest cos
		a = it+1;
		b = precode->begin() + it->jump;

		while(true)
		{ 
			x = std::find_if(a, b, [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoBeginLoop; });

			if(std::find_if_not(a, x, IsArithmeticSafeInstruction) == x)
			{ //mamy ci¹g ar opw
				sum += Calcule(a,x);
				ops += std::count_if(a, x, IsArithmeticInstruction);

				if(x == b) //ostatni
					break;

				a = precode->begin() + x->jump + 1; //pierwsza instrukcja za pomijan¹ pêtl¹
			}
			else return; //sa inne opy, bezzasadne
		}
		
		//ok w tym momecie mamy pewnosc ze po usunieciu wewnêtrznych pêtli mamy same mathopy i niezmieniaj¹ce operacje
		//mamy sume i liczbe opów
		//wyci¹gnijmy wnioski



		if(abs(sum) != ops)
		{
			messages->AddMessage(ParseErrors::ecRedundantLoopArithmetic, it - precode->begin());	
			if(repair)
			{
				//it = precode->erase(it, it+2);
				//++repaired_issues;
				//return;
			}
		}
		
		if(sum > 1)
		{
			messages->AddMessage(ParseErrors::ecSlowLoop, it - precode->begin());	
			if(repair)
			{
				//it = precode->erase(it, it+2);
				//++repaired_issues;
				//return;
			}
		}

		if(ARSearchTool(it))
		{
			messages->AddMessage(ParseErrors::ecRedundantNearLoopArithmetic, it - precode->begin());
		}

		ignore_arithmetic_for_error = true;
	}
}

void Debuger::TestRedundantMoves(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	std::vector<CodeTape::bt_instruction>::iterator n;
	if(IsMoveInstruction(*it))
	{
		ignore_moves_for_error = true;
		n = std::find_if_not(it, precode->end(), IsMoveSafeInstruction);
		//mamy ci¹g < >
		//instrukcji musi byc wiêcej niz jedna i wynik ma byc osi¹gniêty najmniejsza liczba instrukcji, czyli bez np suma = 2 dla >> [ops=2] a nie <><>> [ops=5]
		if((n - it) > 1 && abs(CalculeMoves(it,n)) != std::count_if(it, n, IsMoveInstruction))
		{
			messages->AddMessage(ParseErrors::ecRedundantMoves, it - precode->begin());	
			if(repair)
			{
				//it = precode->erase(it, it+2);
				//++repaired_issues;
				//return;
			}
		}
	}
}


int Debuger::Calcule(std::vector<CodeTape::bt_instruction>::iterator &begin, std::vector<CodeTape::bt_instruction>::iterator &end)
{
	int sum = 0;
	for(std::vector<CodeTape::bt_instruction>::iterator it = begin; it < end; ++it)
	{
		switch(it->operation)
		{
			case CodeTape::btoIncrement: ++sum; break;
			case CodeTape::btoDecrement: --sum; break;
		}
	}

	return sum;
}

int Debuger::CalculeMoves(std::vector<CodeTape::bt_instruction>::iterator &begin, std::vector<CodeTape::bt_instruction>::iterator &end)
{
	int sum = 0;
	for(std::vector<CodeTape::bt_instruction>::iterator it = begin; it < end; ++it)
	{
		switch(it->operation)
		{
			case CodeTape::btoMoveRight: ++sum; break;
			case CodeTape::btoMoveLeft:  --sum; break;
		}
	}

	return sum;
}


//szuka w ty³ wzorca A(?) pocz¹wszy od pocz¹tku pêtli [ np "++}-["
bool Debuger::ARSearchTool(std::vector<CodeTape::bt_instruction>::iterator &begin) 
{
   if(begin - precode->begin() < 2 )
	   return false;

   for(std::vector<CodeTape::bt_instruction>::iterator it = begin - 1; it >= precode->begin(); --it)
   {
		if(IsArithmeticSafeInstruction(*it))
		{
			if(IsArithmeticInstruction(*it))
				return true;
		}
		else return false;
   }
   return false;
}


bool Debuger::isCodeValid(void)
{
	return messages->WarningsCount() == 0;
}

void Debuger::RelinkCommands(std::vector<CodeTape::bt_instruction>::iterator &start, unsigned n)
{
	for(std::vector<CodeTape::bt_instruction>::iterator it = start; it < precode->end(); ++it)
	{
		if(it->NullJump() == false)
		{
			it->jump -= n;
		}
	}
}