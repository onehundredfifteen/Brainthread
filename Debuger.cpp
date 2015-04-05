#include "Debuger.h"

#include <algorithm>
#include <cmath>
#include <iostream>

Debuger::Debuger(MessageLog *messages, std::vector<CodeTape::bt_instruction> *precode, short typesize, bool repair)
{
	language = CodeTape::clBrainThread;
	
	if(messages == nullptr || precode == nullptr)
		throw std::invalid_argument("Debuger::Debuger: invalid argument");

	this->messages = messages;
	this->precode = precode;
	this->repair = repair;
	this->typesize = typesize;

	repaired_issues = 0;

	function_limit = static_cast<unsigned int>(std::pow(2.0,typesize*8));
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
		   op.operation == CodeTape::btoSwap ||
		   op.operation == CodeTape::btoSharedSwap ||
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
		    (op.operation != CodeTape::btoAsciiWrite && op.operation != CodeTape::btoDecimalWrite &&
			 op.operation != CodeTape::btoFork);
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
			 op.operation != CodeTape::btoSharedPush && op.operation != CodeTape::btoPush);
}

bool Debuger::IsLinkedInstruction(const CodeTape::bt_instruction &op)
{
	return (op.operation == CodeTape::btoBeginLoop ||
		   op.operation == CodeTape::btoEndLoop ||
		   op.operation == CodeTape::btoBeginFunction ||
		   op.operation == CodeTape::btoEndFunction); 
}

bool Debuger::IsChangingCellInstruction(const CodeTape::bt_instruction &op)
{
	return (IsArithmeticInstruction(op) ||
		    op.operation == CodeTape::btoPop || op.operation == CodeTape::btoSharedPop ||
			op.operation == CodeTape::btoAsciiRead || op.operation == CodeTape::btoDecimalRead);
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

		TestForFunctionsErrors(it);
		if(c_size != precode->size())
			continue;

		TestForJoinBeforeFork(it);
		if(c_size != precode->size())
			continue;

		TestOpsBeforeFork(it);
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
			messages->AddMessage(MessageLog::ecJoinRepeat, it - precode->begin());

		if(TestForRepetition(it, CodeTape::btoTerminate))
			messages->AddMessage(MessageLog::ecTerminateRepeat, it - precode->begin());

		
		if(IsArithmeticSafeInstruction(*it) == false && ignore_arithmetic_for_error == true) 
			ignore_arithmetic_for_error = false;

		if(IsMoveInstruction(*it) == false && ignore_moves_for_error == true) 
			ignore_moves_for_error = false;

	    ++it;
	}

	if( forks == 0 && joins > 0)
	{
		messages->AddMessage(MessageLog::ecJoinButNoFork, 1);
	}

	if( function_def > function_limit)
	{
		messages->AddMessage(MessageLog::ecFunctionLimitExceed, 1);
	}

	if( function_def > 0 && function_calls == 0)
	{
		messages->AddMessage(MessageLog::ecFunctionExistsButNoCall, 1);
	}

	std::cout << std::endl;
	int q=0;
	for(std::vector<CodeTape::bt_instruction>::iterator it = precode->begin(); it < precode->end(); ++it)
	{
		std::cout << q++ << ": " << it->operation << ": " << (it->NullJump() ? 115 : it->jump) << "\n";
	}
	std::cout << std::endl;
}

//Modelowa funkcja testuj¹ca - je¿eli analizowana komenda jest pocz¹tek pêtli, funkcja
//zacznie swoje zadanie - wykona dwa testy, doda ewentualny b³ad i spróbuje naprawiæ.
//W wypadku bledu funkcja koñczy sie natychmiatowo, aby pêtla, z której by³a wywo³ana
//nie pchnê³a iteratora do przodu.
//Funkcja przyjmuje referencjê iteratora, bo ma nad nim pe³n¹ kontrolê, w przypadku próby
//naprawy iterator bêdzie mia³ to, co zwróci funkcja erase - czyli nastepny element w kolejce,
//który nale¿y przeanalizowaæ. Po naprawie nalezy ponownie powi¹zac komendy (RelinkCommands)
void Debuger::TestForInfiniteLoops(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	if(it->operation == CodeTape::btoBeginLoop)
	{
		//pusta pêtla []
		if( (it + 1)->operation == CodeTape::btoEndLoop)
		{ 
			messages->AddMessage(MessageLog::ecInfiniteLoop, it - precode->begin());	
			if(repair)
			{
				it = precode->erase(it, it+2);
				this->RelinkCommands(it, 2);
				++repaired_issues;
				return;
			}
		}
		//pusta pêtla [[x[x]xx]]
		else if( (it + 1)->operation == CodeTape::btoBeginLoop && (precode->begin() + (it->jump) - 1)->operation == CodeTape::btoEndLoop)
		{
			messages->AddMessage(MessageLog::ecEmptyLoop, it - precode->begin());
			if(repair)
			{
				precode->erase(precode->begin() + (it->jump));
				it = precode->erase(it);
				this->RelinkCommands(it, precode->begin() + (it->jump), 1); //œwiadomie uzywam starej pozycji
				this->RelinkCommands(precode->begin() + (it->jump)+1, 2); //od koñca usuniêtej pêtli juz po dwa
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
				messages->AddMessage(MessageLog::ecFunctionRedefinition, it - precode->begin());
			}
		}
	}
	else if(it->operation == CodeTape::btoBeginFunction) // przypadek ((x)x) + pusta funkcja
	{
		n = precode->begin() + it->jump;//bierzemy ca³¹ treœæ funkcji

		if(n - it == 1) //funkcja jest pusta? ()
		{
			messages->AddMessage(MessageLog::ecEmptyFunction, it - precode->begin());	
			if(repair) //usuwamy funkcjê
			{
				precode->erase(n);
				it = precode->erase(it);
				this->RelinkCommands(it, 2);
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
					messages->AddMessage(MessageLog::ecFunctionRedefinition2, it + 1 - precode->begin());
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
			this->RelinkCommands(it, (n-it-1));
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
			messages->AddMessage(MessageLog::ecJoinBeforeFork, it - precode->begin());	
			if(repair) //usuwamy join
			{
				it = precode->erase(it);
				this->RelinkCommands(it, (n-it-1));
				++repaired_issues;
			}
		}	
	}
}

void Debuger::TestArithmetics(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	std::vector<CodeTape::bt_instruction>::iterator m, n;
	int sum, k, ops;

	if(IsArithmeticInstruction(*it))
	{
		ignore_arithmetic_for_error = true;
		n = std::find_if_not(it, precode->end(), IsArithmeticSafeInstruction);
		//mamy ci¹g + - i instrukcji pomijalnych
		//instrukcji musi byc wiêcej niz jedna i wynik ma byc sosi¹gniêty najmniejsza liczba instrukcji, czyli bez np suma = 2 dla ++ [ops=2] a nie +-+-++ [ops=5]

		if((n - it) > 1 && abs(Calcule(it, n)) != std::count_if(it, n, IsArithmeticInstruction))
		{
			messages->AddMessage(MessageLog::ecRedundantArithmetic, it - precode->begin());	
			if(repair)
			{
				sum = Calcule(it, n);
		        ops = std::count_if(it, n, IsArithmeticInstruction);	
				k = (ops-abs(sum))/2;

				for(int i = 0; i < k; ++i)
				{
					std::find_if(it, n, [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoDecrement; })->operation = CodeTape::btoUnkown;
					std::find_if(it, n, [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoIncrement; })->operation = CodeTape::btoUnkown;
				}

					
				precode->erase(
						std::remove_if(it, precode->end(), [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoUnkown; }),
				precode->end()),
				this->RelinkCommands(it, (ops-sum));
						
				++repaired_issues;
			}
		}
		else if( n != precode->end() && n->operation == CodeTape::btoBeginFunction)
		{
			//specjalny przypadek - musimy jeszcze wyci¹c funkcjê, bo np +++(-)--- to jest bezsensowna operacja i zamienic na +++---
			sum = Calcule(it, n);
		    ops = std::count_if(it, n, IsArithmeticInstruction);

			while(true)
			{ 
				//przeskocz za funkcjê
				m = precode->begin() + n->jump;
				
				//znajdŸ nastêpn¹
				n = std::find_if(m, precode->end(), [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoBeginFunction; });

				//myk z funkcjami w funkcjach
				if(m+1 != precode->end() && n > std::find_if(m+1, precode->end(), [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoEndFunction; }))
					break;

				//zlicz
				sum += Calcule(m, n);
				ops += std::count_if(m, n, IsArithmeticInstruction);

				//nie ma, przerwij
				if(n == precode->end())
					break;
			
			}

			if(abs(sum) != ops)
			{
				messages->AddMessage(MessageLog::ecRedundantArithmetic2, it - precode->begin());	
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
			messages->AddMessage(MessageLog::ecRedundantLoopArithmetic, it - precode->begin());	
		}
		
		if(sum > 1 && typesize > 1)//wolna pêtla dla typów wiêkszych ni¿ 1 bajt
		{
			messages->AddMessage(MessageLog::ecSlowLoop, it - precode->begin());	
			//nienaprawialny
		}

		if(ARSearchTool(it))
		{
			messages->AddMessage(MessageLog::ecRedundantNearLoopArithmetic, it - precode->begin());
		}

		ignore_arithmetic_for_error = true;
	}
}

void Debuger::TestRedundantMoves(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	std::vector<CodeTape::bt_instruction>::iterator n;
	int sum, k, ops;

	if(IsMoveInstruction(*it))
	{
		ignore_moves_for_error = true;
		n = std::find_if_not(it, precode->end(), IsMoveSafeInstruction);
		//mamy ci¹g < >
		//instrukcji musi byc wiêcej niz jedna i wynik ma byc osi¹gniêty najmniejsza liczba instrukcji, czyli bez np suma = 2 dla >> [ops=2] a nie <><>> [ops=5]
		//std::cout <<"fakty " <<(n - it) <<"fakty " <<abs(CalculeMoves(it,n)) <<"fakty " << count_if(it, n, IsMoveInstruction) <<"fakty " << CalculeMoves(it,n)<<"$\n ";
		sum = CalculeMoves(it, n);
		ops = std::count_if(it, n, IsMoveInstruction);
		
		if((n - it) > 1 && abs(sum) != ops)
		{
			messages->AddMessage(MessageLog::ecRedundantMoves, it - precode->begin());	
			if(repair)
			{
				k = (ops-abs(sum))/2;
				for(int i = 0; i < k; ++i)
				{
					std::find_if(it, n, [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoMoveLeft; })->operation = CodeTape::btoUnkown;
					std::find_if(it, n, [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoMoveRight; })->operation = CodeTape::btoUnkown;
				}

				precode->erase(
							std::remove_if(it, precode->end(), [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoUnkown; }),
							precode->end()),
				this->RelinkCommands(it, (ops-sum));
				++repaired_issues;
			}
		}
	}
}

void Debuger::TestOpsBeforeFork(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	std::vector<CodeTape::bt_instruction>::reverse_iterator r;
	if(it->operation == CodeTape::btoFork && it > precode->begin()) 
	{
        r = std::vector<CodeTape::bt_instruction>::reverse_iterator(it);
		if(r != std::find_if_not(r,precode->rend(),IsChangingCellInstruction))
		{
			messages->AddMessage(MessageLog::ecRedundantOpBeforeFork, (r+1).base() - precode->begin());	
		}
	}
}


int Debuger::Calcule(const std::vector<CodeTape::bt_instruction>::iterator &begin, const std::vector<CodeTape::bt_instruction>::iterator &end) const
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

int Debuger::CalculeMoves(const std::vector<CodeTape::bt_instruction>::iterator &begin, const std::vector<CodeTape::bt_instruction>::iterator &end) const
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
bool Debuger::ARSearchTool(const std::vector<CodeTape::bt_instruction>::iterator &begin) 
{
   if(begin == precode->begin())
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


void Debuger::RelinkCommands(const std::vector<CodeTape::bt_instruction>::iterator &start, short n)
{
	RelinkCommands(start, precode->end(), n);
}
void Debuger::RelinkCommands(const std::vector<CodeTape::bt_instruction>::iterator &start, const std::vector<CodeTape::bt_instruction>::iterator &end, short n)
{
	for(std::vector<CodeTape::bt_instruction>::iterator it = start; it < end; ++it)
	{
		if(IsLinkedInstruction(*it) || it->NullJump() == false)
		{
			it->jump -= n;
		}
	}
}

