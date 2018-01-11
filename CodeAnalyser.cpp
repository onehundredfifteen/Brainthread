#include "CodeAnalyser.h"

#include <algorithm>
#include <cmath>
#include <list>

CodeAnalyser::CodeAnalyser(std::vector<CodeTape::bt_instruction> *instructions, bool repair)
{
	language = Parser::clBrainThread;
	
	if(instructions == nullptr)
		throw std::invalid_argument("CodeAnalyser::CodeAnalyser: invalid argument");

	this->instructions = instructions;
	this->repair = repair;
	this->typesize = 1;

	repaired_issues = 0;

	function_limit = static_cast<unsigned int>(std::pow(2.0, typesize*8));
}

CodeAnalyser::~CodeAnalyser(void)
{
}

//Operatory arytmetyczne: + -
bool CodeAnalyser::IsArithmeticInstruction(const CodeTape::bt_instruction &op)
{
	return (op.operation == CodeTape::btoIncrement ||
		    op.operation == CodeTape::btoDecrement ); 
}

//Operatory ruchu piórka: < >
bool CodeAnalyser::IsMoveInstruction(const CodeTape::bt_instruction &op)
{
	return (op.operation == CodeTape::btoMoveLeft ||
		   op.operation == CodeTape::btoMoveRight); 
}

//Operatory zmieniaj¹ce przep³yw lub wartoœæ komórek
//Wszystkie operatory Brainfuck (bez output) + function Call
//Nie ma tutaj definicji funkcji! ()
bool CodeAnalyser::IsChangingInstruction(const CodeTape::bt_instruction &op)
{
	return (IsArithmeticInstruction(op)				  ||
		    IsMoveInstruction(op)					  ||
		    op.operation == CodeTape::btoAsciiRead	  ||
		    op.operation == CodeTape::btoDecimalRead  ||
		    op.operation == CodeTape::btoBeginLoop	  ||
		    op.operation == CodeTape::btoEndLoop      ||
		    op.operation == CodeTape::btoCallFunction ||
		    op.operation == CodeTape::btoPop		  ||
		    op.operation == CodeTape::btoSharedPop);
}

//Operatory ³¹czone parami z innymi: pêtle i funkcje
bool CodeAnalyser::IsLinkedInstruction(const CodeTape::bt_instruction &op)
{
	return (op.operation == CodeTape::btoBeginLoop	   ||
		    op.operation == CodeTape::btoEndLoop	   ||
		    op.operation == CodeTape::btoBeginFunction ||
		    op.operation == CodeTape::btoEndFunction); 
}

//Operatory dla testu Przed forkiem
bool CodeAnalyser::IsChangingCellInstruction(const CodeTape::bt_instruction &op)
{
	return (IsArithmeticInstruction(op) ||
		    op.operation == CodeTape::btoPop || 
			op.operation == CodeTape::btoSharedPop ||
			op.operation == CodeTape::btoAsciiRead || 
			op.operation == CodeTape::btoDecimalRead);
}

//Operatory dla testu Przed forkiem
bool CodeAnalyser::IsSharedHeapInstruction(const CodeTape::bt_instruction &op)
{
	return (op.operation == CodeTape::btoSharedPop || 
			op.operation == CodeTape::btoSharedPush ||
			op.operation == CodeTape::btoSharedSwap);
}

//Operatory ³¹czone parami z innymi: pêtle i funkcje
bool CodeAnalyser::IsFlowChangingInstruction(const CodeTape::bt_instruction &op)
{
	return (IsLinkedInstruction(op) ||
		    op.operation == CodeTape::btoCallFunction); 
}


void CodeAnalyser::Analyse(void)
{
	std::vector<CodeTape::bt_instruction>::iterator it = instructions->begin();
	
	function_def = 0;
	function_calls = 0;
	forks = 0;
	joins = 0;
	ignore_arithmetic_test = false;
	ignore_moves_test = false;
	
	while( it < instructions->end() )
	{
		//reset flag
		if(IsArithmeticInstruction(*it) == false && ignore_arithmetic_test == true) 
			ignore_arithmetic_test = false;

		if(IsMoveInstruction(*it) == false && ignore_moves_test == true) 
			ignore_moves_test = false;

		//testy
		if(TestForInfiniteLoops(it))
			continue;

		if(TestForFunctionsErrors(it))
			continue;

		if(TestForThreads(it))
			continue;

		if(TestForHeaps(it))
			continue;

		if(TestLoopPerformance(it))
			continue;

		if(ignore_arithmetic_test == false)
		{
			if(TestArithmetics(it))
			  continue;
		}

		if(ignore_moves_test == false)
		{
			if(TestRedundantMoves(it))
				continue;
		}

		if(it->operation == CodeTape::btoFork) 
			++forks;
		else if(it->operation == CodeTape::btoJoin) 
			++joins;
		else if(it->operation == CodeTape::btoCallFunction) 
			++function_calls;
	
	    ++it;
	}

	//dodatkowe b³êdy statystyczne
	if( forks == 0 && joins > 0)
	{
		MessageLog::Instance().AddMessage(MessageLog::ecJoinButNoFork, 1);
	}

	if( function_def > function_limit)
	{
		MessageLog::Instance().AddMessage(MessageLog::ecFunctionLimitExceed, 1);
	}

	if( function_def > 0 && function_calls == 0)
	{
		MessageLog::Instance().AddMessage(MessageLog::ecFunctionExistsButNoCall, 1);
	}

	if(TestLinks())
	{
		MessageLog::Instance().AddMessage(MessageLog::ecIntegrityLost, 1);
	}

	/*std::cout << std::endl;
	int q=0;
	for(std::vector<CodeTape::bt_instruction>::iterator it = instructions->begin(); it < instructions->end(); ++it)
	{
		std::cout << q++ << ": " << it->operation << ": " << (it->NullJump() ? 115 : it->jump) << "\n";
	}
	std::cout << std::endl;*/
}

//Modelowa funkcja testuj¹ca - je¿eli analizowana komenda jest pocz¹tek pêtli, funkcja
//zacznie swoje zadanie - wykona dwa testy, doda ewentualny b³ad i spróbuje naprawiæ.
//W wypadku bledu funkcja koñczy sie natychmiatowo, aby pêtla, z której by³a wywo³ana
//nie pchnê³a iteratora do przodu.
//Funkcja przyjmuje referencjê iteratora, bo ma nad nim pe³n¹ kontrolê, w przypadku próby
//naprawy iterator bêdzie mia³ to, co zwróci funkcja erase - czyli nastepny element w kolejce,
//który nale¿y przeanalizowaæ. Po naprawie nalezy ponownie powi¹zac komendy (RelinkCommands)
//Funkcja zwraca prawde, je¿eli naprawila coœ
//1. Puste pêtle
//2. Nieskoñczone pêtle
bool CodeAnalyser::TestForInfiniteLoops(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	if(it->operation == CodeTape::btoBeginLoop)
	{
		//nieskoñczona pêtla []
		if( (it + 1)->operation == CodeTape::btoEndLoop)
		{ 
			MessageLog::Instance().AddMessage(MessageLog::ecInfiniteLoop, it - instructions->begin() + 1);	
			if(repair)
			{
				it = instructions->erase(it, it+2);
				this->RelinkCommands(it, 2);
				++repaired_issues;
				return true;
			}
		}
		//pusta pêtla [[xxxx]]
		else if( (it + 1)->operation == CodeTape::btoBeginLoop && (instructions->begin() + (it->jump) - 1)->operation == CodeTape::btoEndLoop)
		{
			MessageLog::Instance().AddMessage(MessageLog::ecEmptyLoop, it - instructions->begin() + 1);
			if(repair)
			{
				instructions->erase(instructions->begin() + (it->jump));
				it = instructions->erase(it);
				this->RelinkCommands(it, instructions->begin() + (it->jump), 1); //œwiadomie uzywam starej pozycji
				this->RelinkCommands(instructions->begin() + (it->jump)+1, 2); //od koñca usuniêtej pêtli juz po dwa
				++repaired_issues;
				return true;
			}
		}	 
	}

	return false;
}

//Funkcja sprawdza brainfakowe metody:
//1. Puste metody - 2 rodzaje - () i ((xx))
//2. Redefinicja (xx)(xx)
//3. Rekurencja (:xx) 
//4. Funkcja w pêtli 
bool CodeAnalyser::TestForFunctionsErrors(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	std::vector<CodeTape::bt_instruction>::iterator m, n, o;
	std::vector<CodeTape::bt_instruction>::reverse_iterator r, s;

	if(it->operation == CodeTape::btoEndFunction)
	{
		//szukam nastêpnej funkcji
		n = std::find_if(it, instructions->end(), 
			             [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoBeginFunction; });
			
		if(n != instructions->end()) //jest - sprawdzamy instrukcje pomiedzy ) i (
		{	
			m = std::find_if(it, n, IsChangingInstruction);
			if(m == n || n - it == 1) //nie ma zmieniajacych coœ instrukcji lub w ogóle nie ma nic
			{
				//jest podejrzenie redefinicji, bo nie ma instrukcji zmieniaj¹cych wartoœæ miêdzy funkcjami
				MessageLog::Instance().AddMessage(MessageLog::ecFunctionRedefinition, n - instructions->begin() + 1);
			}
		}
	}
	else if(it->operation == CodeTape::btoBeginFunction) 
	{
		n = instructions->begin() + it->jump;//bierzemy ca³¹ treœæ funkcji

		//pusta funkcja  ()
		if((it + 1)->operation == CodeTape::btoEndFunction) 
		{
			MessageLog::Instance().AddMessage(MessageLog::ecEmptyFunction, it - instructions->begin() + 1);	
			if(repair) //usuwamy funkcjê
			{
				instructions->erase(n);
				it = instructions->erase(it);
				this->RelinkCommands(it, 2);
				++repaired_issues;
				return true;
			}
		}
		//pusta funkcja ((xx))
		else if( (it + 1)->operation == CodeTape::btoBeginFunction && (instructions->begin() + (it->jump) - 1)->operation == CodeTape::btoEndFunction)
		{
			MessageLog::Instance().AddMessage(MessageLog::ecEmptyFunction, it - instructions->begin() + 1);
			if(repair)
			{
				instructions->erase(instructions->begin() + (it->jump));
				it = instructions->erase(it);
				this->RelinkCommands(it, instructions->begin() + (it->jump), 1); //œwiadomie uzywam starej pozycji
				this->RelinkCommands(instructions->begin() + (it->jump)+1, 2); //od koñca usuniêtej pêtli juz po dwa
				++repaired_issues;
				return true;
			}
		}	 
		else // nie jest pusta -> szukamy funkcji wewêtrznej (bez pierwszego elementu, bo to wlasnie '(' )
		{
			m = std::find_if(it + 1, n, 
				             [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoBeginFunction; });

			if(m != n) //mamy funkcjê wewnêtrzn¹
			{
				++function_limit; //arbitralnie podnosimy limit funkcji

				o = std::find_if(it, m, IsChangingInstruction); //szukamy czy dziel¹ je jakies istotne instrukcje
				if( o == m || m - it == 1) //jest podejrzenie redefinicji, bo nie ma instrukcji zmieniaj¹cych wartoœæ miêdzy funkcjami (xx( lub pusto ((
				{
					MessageLog::Instance().AddMessage(MessageLog::ecFunctionRedefinitionInternal, m - instructions->begin() + 1);
				}
			}
			else //nie ma funkcji wewnêtrznaj
			{
				m = std::find_if(it + 1, n, 
					             [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoCallFunction; });

				o = std::find_if(it + 1, m, IsChangingInstruction); //szukamy zmieniajacych instrukcji do momentu pierwszego call, nastpne calle niewazne
				if( m != n && o == m)  //jest call i nie ma instrukcji zmieniajacych 
			    {
					MessageLog::Instance().AddMessage(MessageLog::ecInfinityRecurention, m - instructions->begin() + 1);
				}
			}

		}
		
		++function_def;	
	}
	else if(it->operation == CodeTape::btoBeginLoop) // funkcja w pêtli
	{
		n = instructions->begin() + it->jump;

		m = std::find_if(it + 1, n, 
					             [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoBeginFunction; });

		if(m != n)
		{
			MessageLog::Instance().AddMessage(MessageLog::ecFunctionInLoop, m - instructions->begin() + 1);
		}
	}
	else if(it->operation == CodeTape::btoCallFunction) //undefined call
	{
		r = std::vector<CodeTape::bt_instruction>::reverse_iterator(it);
		s = std::find_if(r, instructions->rend(), 
								[](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoBeginFunction; });
		
		if(s == instructions->rend())
		{
			MessageLog::Instance().AddMessage(MessageLog::ecCallButNoFunction, (r+1).base() - instructions->begin());	
		}
	}
	return false;
}

//Funkcja testuje sprawy zwi¹zane z w¹tkami
//1. Join, czy nie jest za wczeœnie wywo³any
//2. Czy ktoœ zapomnia³, ¿e fork zeruje bie¿¹ca komórke i cos tam dodawa³ odejmowa³ wczesniej
//3 i 4 - powtórzenia join i terminate
bool CodeAnalyser::TestForThreads(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	std::vector<CodeTape::bt_instruction>::iterator n, m;
	std::vector<CodeTape::bt_instruction>::reverse_iterator r;

	if(it->operation == CodeTape::btoJoin)
	{
		if(TestForRepetition(it, CodeTape::btoJoin))
			MessageLog::Instance().AddMessage(MessageLog::ecJoinRepeat, it - instructions->begin() + 1);
		
		
		if(IsWithinFunction(it) == false && forks == 0) //join poza funkcj¹. Mo¿e byc call do póŸniejszej funkcji z fork, ale to trudno stwierdziæ
		{
			MessageLog::Instance().AddMessage(MessageLog::ecJoinBeforeFork, it - instructions->begin() + 1);	
			if(repair) //usuwamy join
			{
				it = instructions->erase(it);
				this->RelinkCommands(it, (n - it - 1));
				++repaired_issues;
				return true;
			}
		}	
	}
	else if(it->operation == CodeTape::btoFork && it > instructions->begin()) 
	{
        r = std::vector<CodeTape::bt_instruction>::reverse_iterator(it);
		if(r != std::find_if_not(r, instructions->rend(), IsChangingCellInstruction))
		{
			MessageLog::Instance().AddMessage(MessageLog::ecRedundantOpBeforeFork, (r+1).base() - instructions->begin());	
		}
	}
	else if(it->operation == CodeTape::btoTerminate) 
	{
        if(TestForRepetition(it, CodeTape::btoTerminate))
			MessageLog::Instance().AddMessage(MessageLog::ecTerminateRepeat, it - instructions->begin() + 1);
	}
	
	return false;
}

//Funkcja testuje sprawy zwi¹zane z heap
//Szczegolnie switche [sa dystalne]
//1. Switch scope and repeat
//2. swap repeat
bool CodeAnalyser::TestForHeaps(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	std::vector<CodeTape::bt_instruction>::iterator n, m;
	std::vector<CodeTape::bt_instruction>::reverse_iterator r;

	if(it->operation == CodeTape::btoSwitchHeap)
	{
		if(TestForRepetition(it, CodeTape::btoSwitchHeap))
			MessageLog::Instance().AddMessage(MessageLog::ecSwitchRepeat, it - instructions->begin() + 1);

		n = std::find_if(it + 1, instructions->end(), IsSharedHeapInstruction);

		if(n == instructions->end()) 
		{
			MessageLog::Instance().AddMessage(MessageLog::ecRedundantSwitch, it - instructions->begin() + 1);

			if(repair) //usuwamy switch
			{
				it = instructions->erase(it);
				this->RelinkCommands(it, (n - it - 1));
				++repaired_issues;
				return true;
			}
		}
		else // jest jakas instrukcja shared heapa
		{
			m = std::find_if(it + 1, n, IsFlowChangingInstruction);

			if(m != n) 
				MessageLog::Instance().AddMessage(MessageLog::ecSwithOutOfScope, it - instructions->begin() + 1);
		}
	}
	else if(it->operation == CodeTape::btoSwap) 
	{
        if(TestForRepetition(it, CodeTape::btoSwap))
			MessageLog::Instance().AddMessage(MessageLog::ecSwapRepeat, it - instructions->begin() + 1);
	}
	else if(it->operation == CodeTape::btoSharedSwap) 
	{   //we have ~ op beetween
        n = std::find_if(it + 1, instructions->end(), 
			             [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoSharedSwap; });

		if(n != instructions->end() && n - it == 2) //~%~%
		{
			MessageLog::Instance().AddMessage(MessageLog::ecSwapRepeat, it - instructions->begin() + 1);
		}
	}
	return false;
}

//Funkcja testuje pêtle, czy:
//1. wszystkie grzecznie szybko d¹¿¹ do zera
//2. Nie ma niepotrzebnych operacji + - przed
bool CodeAnalyser::TestLoopPerformance(std::vector<CodeTape::bt_instruction>::iterator &it)
{   
	std::vector<CodeTape::bt_instruction>::iterator n;
	int lim, s;

	if(it->operation == CodeTape::btoBeginLoop)
	{
		lim = GetLoopLimes(it);

		if(lim > 0) //wolna pêtla d¹¿¹ca do nieskoñczonoœci
		{
			MessageLog::Instance().AddMessage(MessageLog::ecSlowLoop, it - instructions->begin() + 1);	
		}

		//teraz czy przed pêtl¹ s¹ sensowne operacje
		if(it > instructions->begin()) //ofc tylko dla dalszych instrukcji
		{
			for(n = it - 1; n > instructions->begin(); --n)
			{
				if(IsArithmeticInstruction(*n) == false)
				   break; //krêci siê w ty³, a¿ znajdzie inna instrukcje + - albo pocz¹tek ci¹gu
			}

			if(it - 1 != n) //jest znaleziona przynajmniej jedna instrukcja + -
			{
				//tutaj n oznacza pierwsza nie + - instrukcje przed pêtl¹
				s = Evaluate(n + 1, it);

				if(lim != 0 && (lim > 0 || (lim < 0 && s < 0) )) //je¿eli pêtla jest policzalna, a sekwencja przed ni¹ da¿y inaczej ni¿ pêtla
				{
					MessageLog::Instance().AddMessage(MessageLog::ecRedundantNearLoopArithmetic, it - instructions->begin() ); // specjalnie bez  + 1
				}
			}
		}
 
	}
	return false;
}

//Funkcja testuje operacje arytmetyczne, czy nie sa nadmiarowe
bool CodeAnalyser::TestArithmetics(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	std::vector<CodeTape::bt_instruction>::iterator m, n;
	int sum, ops, k;

	if(IsArithmeticInstruction(*it))
	{
		ignore_arithmetic_test = true;
		n = std::find_if_not(it, instructions->end(), IsArithmeticInstruction);
		//mamy ci¹g + - 
		//instrukcji musi byc wiêcej niz jedna i wynik ma byc osi¹gniêty najmniejsza liczba instrukcji, czyli bez np suma = 2 dla ++ [ops=2] a nie +-+-++ [ops=5]

		ops = std::count_if(it, n, IsArithmeticInstruction);
		sum = Evaluate(it, n);

		if((n - it) > 1 && abs(sum) != ops)
		{
			MessageLog::Instance().AddMessage(MessageLog::ecRedundantArithmetic, it - instructions->begin() + 1);	
			if(repair)
			{
				k = (ops - abs(sum)) / 2;

				for(int i = 0; i < k; ++i)
				{
					std::find_if(it, n, [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoDecrement; })->operation = CodeTape::btoUnkown;
					std::find_if(it, n, [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoIncrement; })->operation = CodeTape::btoUnkown;
				}
					
				instructions->erase(
						std::remove_if(it, instructions->end(), [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoUnkown; }),
				instructions->end()),
				this->RelinkCommands(it, (ops - sum));
						
				++repaired_issues;
				return true;
			}
		}
	}

	return false;
}

//Funkcja testuje operacje ruchu, czy nie sa nadmiarowe
bool CodeAnalyser::TestRedundantMoves(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	std::vector<CodeTape::bt_instruction>::iterator n;
	int sum, k, ops;

	if(IsMoveInstruction(*it))
	{
		ignore_moves_test = true;
		n = std::find_if_not(it, instructions->end(), IsMoveInstruction);
		//mamy ci¹g < > 
		//instrukcji musi byc wiêcej niz jedna i wynik ma byc osi¹gniêty najmniejsza liczba instrukcji, czyli bez np suma = 2 dla >> [ops=2] a nie <><>>> [ops=5]

		sum = EvaluateMoves(it, n);
		ops = std::count_if(it, n, IsMoveInstruction);
		
		if((n - it) > 1 && abs(sum) != ops)
		{
			MessageLog::Instance().AddMessage(MessageLog::ecRedundantMoves, it - instructions->begin() + 1);	
			if(repair)
			{
				k = (ops-abs(sum))/2;
				for(int i = 0; i < k; ++i)
				{
					std::find_if(it, n, [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoMoveLeft; })->operation = CodeTape::btoUnkown;
					std::find_if(it, n, [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoMoveRight; })->operation = CodeTape::btoUnkown;
				}

				instructions->erase(
							std::remove_if(it, instructions->end(), [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoUnkown; }),
							instructions->end()),
				this->RelinkCommands(it, (ops-sum));
				++repaired_issues;

				return true;
			}
		}
	}

	return false;
}

//Funkcja testuje czy wystapi³y jakies powtórzenia operatorów, np !
bool CodeAnalyser::TestForRepetition(std::vector<CodeTape::bt_instruction>::iterator &it, const CodeTape::bt_operation &op)
{
	std::vector<CodeTape::bt_instruction>::iterator n;

	if(it->operation == op && it + 1 < instructions->end() && (it+1)->operation == op)
	{
		n = std::find_if_not(it + 1, instructions->end(), [&op](const CodeTape::bt_instruction &o){ return o.operation == op; });
		if(n != instructions->end()) //jest jakies powtórzenie
		{
			if(repair) //usuwamy wszsytkie bez pierwszego
			{
				this->RelinkCommands(it, (n-it-1));
				it = instructions->erase(it+1, n);
				++repaired_issues;
			}
			else it = n - 1;
		}
	}
	else return false;

	return true;
}

//Funkcja liczy sume wynikaj¹ca z operacji + i - w pewnym ci¹gu instrukcji od start do poprzedzaj¹cego end
//Nie sa wa¿ne inne operacje 
int CodeAnalyser::Evaluate(const std::vector<CodeTape::bt_instruction>::iterator &begin, const std::vector<CodeTape::bt_instruction>::iterator &end) const
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

//Funkcja liczy operacje < i >, cel taki jak w Calcule
//Nie sa wa¿ne inne operacje 
int CodeAnalyser::EvaluateMoves(const std::vector<CodeTape::bt_instruction>::iterator &begin, const std::vector<CodeTape::bt_instruction>::iterator &end) const
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

//Funkcja liczy sposób zachowania siê pêtli
//Return 0 - niepoliczalne, -1: pêtla d¹¿y do zera, 1: pêtla d¹¿y do nieskoñczonoœci
short CodeAnalyser::GetLoopLimes(const std::vector<CodeTape::bt_instruction>::iterator &op) const
{
	std::vector<CodeTape::bt_instruction>::iterator a, n, m;
	std::list<short> limesii; //wyniki podpêtli w kolejnoœci (to wa¿ne jaka kolejnoœæ)
	int s = 0;

	a = instructions->begin() + op->jump;
	m = op + 1;

	if(m == a)
		return 0; //nieskoñczona pêtla

	n = std::find_if_not(m, a, [&op](const CodeTape::bt_instruction &o){ return IsArithmeticInstruction(o) || 
																				IsChangingInstruction(o) == false ||
																				o.operation == CodeTape::btoBeginLoop || 
																				o.operation == CodeTape::btoEndLoop; } );

	if(n == a) //s¹ tylko + - i pêtle i inne niewazne operacje
	{
		while(true)
		{
			n = std::find_if(m, a, [&op](const CodeTape::bt_instruction &o){ return o.operation == CodeTape::btoBeginLoop || 
																					 o.operation == CodeTape::btoEndLoop; }); //szukamy pierwszej instrukcji pêtli

			if(n != m) //sa jakies operacje -> [+-+-[
				limesii.push_back( Evaluate(m, a) < 0 ? -1 : 1 ); //pêtla z przewaga minusów da tutaj -1, inaczej 1, zero liczy sie jak do nieksonczonoœci
			else   //nie ma ¿adnej operacji -> [[
				limesii.push_back( -1 ); //liczy sie jako normalna zdrowa operacje d¹¿¹ca do zera

			if(n != a) //oho mamy pêtle w œrodku jak¹œ, dawaj jeszcze jej limes
			{
				limesii.push_back(GetLoopLimes(n));
				m = instructions->begin() + n->jump + 1;
			}
			else break; //nie ma pêtli wewnêtrznej
		}
		//mamy tutaj ci¹g 1 i -1 , cos spróbujemy policzyæ

		for(std::list<short>::iterator it = limesii.begin(); it != limesii.end(); ++it)
		{
			if(*it == 0) //którykolwiek element niepoliczalny wyklucza wszystko
				return 0;
		}

		return limesii.back() < 0 ? -1 : 1; //gdy ostatnie d¹¿y w górê do nieskonczonoœci, na pewno siê coœ zjebie 
	}
	
	return 0; // nie da siê policzyæ limes
}


//Funkcja zwraca true, je¿eli ¿¹dana instrukcja znajduje siê w œrodku funkcji
//Liczymy poprostu nawiasy, inaczej nie bêdziemy pewni
bool CodeAnalyser::IsWithinFunction(const std::vector<CodeTape::bt_instruction>::iterator &op) const
{
	int opening_bracket_cnt, closing_bracket_cnt;

	opening_bracket_cnt = std::count_if(instructions->begin(), op, 
		                                [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoBeginFunction; });

	closing_bracket_cnt = std::count_if(instructions->begin(), op, 
		                                [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoEndFunction; });

	return opening_bracket_cnt > closing_bracket_cnt;
}

//Funkcje modyfikuj¹ linkowania 
void CodeAnalyser::RelinkCommands(const std::vector<CodeTape::bt_instruction>::iterator &start, short n)
{
	RelinkCommands(start, instructions->end(), n);
}

void CodeAnalyser::RelinkCommands(const std::vector<CodeTape::bt_instruction>::iterator &start, const std::vector<CodeTape::bt_instruction>::iterator &end, short n)
{
	for(std::vector<CodeTape::bt_instruction>::iterator it = start; it < end; ++it)
	{
		if(IsLinkedInstruction(*it) || it->NullJump() == false)
		{
			it->jump -= n;
		}
	}
}

//Testuje poprawnoœæ linkujacych komend - ka¿da z nich musi linkowaæ do innej, nie mog¹ byc bez par
bool CodeAnalyser::TestLinks()
{
	for(std::vector<CodeTape::bt_instruction>::iterator it = instructions->begin(); it < instructions->end(); ++it)
	{
		if(IsLinkedInstruction(*it) && IsLinkedInstruction(instructions->at( it->jump ) ) == false)
		{
			return true;
		}
	}

	return false;
}

//kod jest zdrowy?
bool CodeAnalyser::isCodeValid(void)
{
	return MessageLog::Instance().WarningsCount() == 0;
}

//czy cos naprawiono?
bool CodeAnalyser::RepairedSomething(void)
{
	return repaired_issues > 0;
}

/*zapasowa dzialaj¹ca dla [+-] bez innych instrukcji
short CodeAnalyser::GetLoopLimes(const std::vector<CodeTape::bt_instruction>::iterator &op) const
{
	std::vector<CodeTape::bt_instruction>::iterator a, n, m;
	std::list<short> limesii; //wyniki podpêtli w kolejnoœci (to wa¿ne jaka kolejnoœæ)
	int s = 0;

	a = instructions->begin() + op->jump;
	m = op + 1;

	if(m == a)
		return 0; //nieskoñczona pêtla

	n = std::find_if_not(m, a, [&op](const CodeTape::bt_instruction &o){ return IsArithmeticInstruction(o) ||
																					 o.operation == CodeTape::btoBeginLoop || 
																					 o.operation == CodeTape::btoEndLoop; } );

	if(n == a) //s¹ tylko + - i pêtle
	{
		while(true)
		{
			n = std::find_if_not(m, a, IsArithmeticInstruction); //szukamy pierwszej instrukcji nie + -, czyli pêtli

			if(n != m) //sa jakies operacje -> [+-+-[
				limesii.push_back( Calcule(m, a) < 0 ? -1 : 1 ); //pêtla z przewaga minusów da tutaj -1, inaczej 1, zero liczy sie jak do nieksonczonoœci
			else   //nie ma ¿adnej operacji -> [[
				limesii.push_back( -1 ); //liczy sie jako normalna zdrowa operacje d¹¿¹ca do zera

			if(n != a) //oho mamy pêtle w œrodku jak¹œ, dawaj jeszcze jej limes
			{
				limesii.push_back(GetLoopLimes(n));
				m = instructions->begin() + n->jump + 1;
			}
			else break; //nie ma pêtli wewnêtrznej
		}
		//mamy tutaj ci¹g 1 i -1 , cos spróbujemy policzyæ

		for(std::list<short>::iterator it = limesii.begin(); it != limesii.end(); ++it)
		{
			if(*it == 0) //którykolwiek element niepoliczalny wyklucza wszystko
				return 0;
		}

		return limesii.back() < 0 ? -1 : 1; //gdy ostatnie d¹¿y w górê do nieskonczonoœci, na pewno siê coœ zjebie 
	}
	
	return 0; // nie da siê policzyæ limes
}
*/
