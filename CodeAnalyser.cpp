#include "CodeAnalyser.h"

#include <algorithm>
#include <cmath>
#include <list>

CodeAnalyser::CodeAnalyser(std::vector<CodeTape::bt_instruction> *instructions)
	: language(Parser::clBrainThread), typesize(1)
{
	if(instructions == nullptr)
		throw std::invalid_argument("CodeAnalyser::CodeAnalyser: invalid argument");

	this->instructions = instructions;
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

//Operatory ruchu pi�rka: < >
bool CodeAnalyser::IsMoveInstruction(const CodeTape::bt_instruction &op)
{
	return (op.operation == CodeTape::btoMoveLeft ||
		   op.operation == CodeTape::btoMoveRight); 
}

//Operatory zmieniaj�ce przep�yw lub warto�� kom�rek
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

//Operatory ��czone parami z innymi: p�tle i funkcje
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

//Operatory ��czone parami z innymi: p�tle i funkcje
bool CodeAnalyser::IsFlowChangingInstruction(const CodeTape::bt_instruction &op)
{
	return (IsLinkedInstruction(op) ||
		    op.operation == CodeTape::btoCallFunction); 
}


void CodeAnalyser::Analyse(void)
{
	CodeIterator it = instructions->begin();
	
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
		
		if (TestForInfiniteLoops(it))
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

	//dodatkowe b��dy statystyczne
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

void CodeAnalyser::Repair(void)
{
	CodeIterator it = instructions->begin();

	function_def = 0;
	function_calls = 0;
	forks = 0;
	joins = 0;
	ignore_arithmetic_test = false;
	ignore_moves_test = false;

	//funkcje naprawcze
	auto repairRepetition = [this](CodeIterator &_it, CodeIterator &n) {
		this->RelinkCommands(_it, (n - _it - 1));
		_it = instructions->erase(_it + 1, n);
		++repaired_issues;
		return false;
	};

	auto repairH = [this](CodeIterator &_it, CodeIterator &n) {
		_it = instructions->erase(_it);
		this->RelinkCommands(_it, (n - _it - 1));
		++repaired_issues;
		return true; 
	};

	while (it < instructions->end())
	{
		//reset flag
		if (IsArithmeticInstruction(*it) == false && ignore_arithmetic_test == true)
			ignore_arithmetic_test = false;

		if (IsMoveInstruction(*it) == false && ignore_moves_test == true)
			ignore_moves_test = false;

		//testy

		

		if (TestForInfiniteLoops(it,
			[this](CodeIterator &_it) {
				_it = instructions->erase(_it, _it + 2);
				RelinkCommands(_it, 2);
				++repaired_issues;
			},
			[this](CodeIterator &_it) {
				instructions->erase(instructions->begin() + (_it->jump));
				_it = instructions->erase(_it);
				RelinkCommands(_it, instructions->begin() + (_it->jump), 1); //�wiadomie uzywam starej pozycji
				RelinkCommands(instructions->begin() + (_it->jump) + 1, 2); //od ko�ca usuni�tej p�tli juz po dwa
				++repaired_issues;
			}))
			continue;

		if (TestForFunctionsErrors(it,
			[this](CodeIterator &_it, CodeIterator &n) {
			instructions->erase(n);
			_it = instructions->erase(_it);
			this->RelinkCommands(_it, 2);
			++repaired_issues;
		},
			[this](CodeIterator &_it) {
			instructions->erase(instructions->begin() + (_it->jump));
			_it = instructions->erase(_it);
			this->RelinkCommands(_it, instructions->begin() + (_it->jump), 1); //�wiadomie uzywam starej pozycji
			this->RelinkCommands(instructions->begin() + (_it->jump) + 1, 2); //od ko�ca usuni�tej p�tli juz po dwa
			++repaired_issues;
		}))
			continue;

		if (TestForThreads(it, repairH, repairRepetition))
			continue;

		if (TestForHeaps(it, repairH, repairRepetition))
			continue;

		if (TestLoopPerformance(it))
			continue;

		if (ignore_arithmetic_test == false)
		{
			if (TestArithmetics(it,
				[this](CodeIterator &_it, CodeIterator &n, int &sum, int &ops) {
				int k = (ops - abs(sum)) / 2;

				for (int i = 0; i < k; ++i)
				{
					std::find_if(_it, n, [](const CodeTape::bt_instruction &op) { return op.operation == CodeTape::btoDecrement; })->operation = CodeTape::btoUnkown;
					std::find_if(_it, n, [](const CodeTape::bt_instruction &op) { return op.operation == CodeTape::btoIncrement; })->operation = CodeTape::btoUnkown;
				}

				instructions->erase(
					std::remove_if(_it, instructions->end(), [](const CodeTape::bt_instruction &op) { return op.operation == CodeTape::btoUnkown; }),
					instructions->end()),
					this->RelinkCommands(_it, (ops - sum));

				++repaired_issues;
			}))
				continue;
		}

		if (ignore_moves_test == false)
		{
			if (TestRedundantMoves(it,
				[this](CodeIterator &_it, CodeIterator &n, int &sum, int &ops) {
				int k = (ops - abs(sum)) / 2;
				for (int i = 0; i < k; ++i)
				{
					std::find_if(_it, n, [](const CodeTape::bt_instruction &op) { return op.operation == CodeTape::btoMoveLeft; })->operation = CodeTape::btoUnkown;
					std::find_if(_it, n, [](const CodeTape::bt_instruction &op) { return op.operation == CodeTape::btoMoveRight; })->operation = CodeTape::btoUnkown;
				}

				instructions->erase(
					std::remove_if(_it, instructions->end(), [](const CodeTape::bt_instruction &op) { return op.operation == CodeTape::btoUnkown; }),
					instructions->end()),
					this->RelinkCommands(_it, (ops - sum));
				++repaired_issues;
			}))
				continue;
		}

		if (it->operation == CodeTape::btoFork)
			++forks;
		else if (it->operation == CodeTape::btoJoin)
			++joins;
		else if (it->operation == CodeTape::btoCallFunction)
			++function_calls;

		++it;
	}

	//dodatkowe b��dy statystyczne
	if (forks == 0 && joins > 0)
	{
		MessageLog::Instance().AddMessage(MessageLog::ecJoinButNoFork, 1);
	}

	if (function_def > function_limit)
	{
		MessageLog::Instance().AddMessage(MessageLog::ecFunctionLimitExceed, 1);
	}

	if (function_def > 0 && function_calls == 0)
	{
		MessageLog::Instance().AddMessage(MessageLog::ecFunctionExistsButNoCall, 1);
	}

	if (TestLinks())
	{
		MessageLog::Instance().AddMessage(MessageLog::ecIntegrityLost, 1);
	}
}


//Modelowa funkcja testuj�ca - je�eli analizowana komenda jest pocz�tek p�tli, funkcja
//zacznie swoje zadanie - wykona dwa testy, doda ewentualny b�ad i spr�buje naprawi�.
//W wypadku bledu funkcja ko�czy sie natychmiatowo, aby p�tla, z kt�rej by�a wywo�ana
//nie pchn�a iteratora do przodu.
//Funkcja przyjmuje referencj� iteratora, bo ma nad nim pe�n� kontrol�, w przypadku pr�by
//naprawy iterator b�dzie mia� to, co zwr�ci funkcja erase - czyli nastepny element w kolejce,
//kt�ry nale�y przeanalizowa�. Po naprawie nalezy ponownie powi�zac komendy (RelinkCommands)
//Funkcja zwraca prawde, je�eli naprawila co�
//1. Puste p�tle
//2. Niesko�czone p�tle
bool CodeAnalyser::TestForInfiniteLoops(CodeIterator &it, const RepairFn &infLoopRep, const RepairFn &emptyLoopRep)
{
	if(it->operation == CodeTape::btoBeginLoop)
	{
		//niesko�czona p�tla []
		if( (it + 1)->operation == CodeTape::btoEndLoop)
		{ 
			MessageLog::Instance().AddMessage(MessageLog::ecInfiniteLoop, it - instructions->begin() + 1);	
			if(infLoopRep)
			{
				infLoopRep(it);			
			}
		}
		//pusta p�tla [[xxxx]]
		else if( (it + 1)->operation == CodeTape::btoBeginLoop && (instructions->begin() + (it->jump) - 1)->operation == CodeTape::btoEndLoop)
		{
			MessageLog::Instance().AddMessage(MessageLog::ecEmptyLoop, it - instructions->begin() + 1);
			if(emptyLoopRep)
			{
				emptyLoopRep(it);
			}
		}	 
	}

	return false;
}

//Funkcja sprawdza brainfakowe metody:
//1. Puste metody - 2 rodzaje - () i ((xx))
//2. Redefinicja (xx)(xx)
//3. Rekurencja (:xx) 
//4. Funkcja w p�tli 
bool CodeAnalyser::TestForFunctionsErrors(CodeIterator &it, const RepairFn2 &emptyFunType1Rep, const RepairFn &emptyFunType2Rep)
{
	CodeIterator m, n, o;
	std::vector<CodeTape::bt_instruction>::reverse_iterator r, s;

	if(it->operation == CodeTape::btoEndFunction)
	{
		//szukam nast�pnej funkcji
		n = std::find_if(it, instructions->end(), 
			             [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoBeginFunction; });
			
		if(n != instructions->end()) //jest - sprawdzamy instrukcje pomiedzy ) i (
		{	
			m = std::find_if(it, n, IsChangingInstruction);
			if(m == n || n - it == 1) //nie ma zmieniajacych co� instrukcji lub w og�le nie ma nic
			{
				//jest podejrzenie redefinicji, bo nie ma instrukcji zmieniaj�cych warto�� mi�dzy funkcjami
				MessageLog::Instance().AddMessage(MessageLog::ecFunctionRedefinition, n - instructions->begin() + 1);
			}
		}
	}
	else if(it->operation == CodeTape::btoBeginFunction) 
	{
		n = instructions->begin() + it->jump;//bierzemy ca�� tre�� funkcji

		//pusta funkcja  ()
		if((it + 1)->operation == CodeTape::btoEndFunction) 
		{
			MessageLog::Instance().AddMessage(MessageLog::ecEmptyFunction, it - instructions->begin() + 1);	
			if(emptyFunType1Rep) //usuwamy funkcj�
			{
				emptyFunType1Rep(it, n);
			}
		}
		//pusta funkcja ((xx))
		else if( (it + 1)->operation == CodeTape::btoBeginFunction && (instructions->begin() + (it->jump) - 1)->operation == CodeTape::btoEndFunction)
		{
			MessageLog::Instance().AddMessage(MessageLog::ecEmptyFunction, it - instructions->begin() + 1);
			if(emptyFunType2Rep)
			{
				emptyFunType2Rep(it);
			}
		}	 
		else // nie jest pusta -> szukamy funkcji wew�trznej (bez pierwszego elementu, bo to wlasnie '(' )
		{
			m = std::find_if(it + 1, n, 
				             [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoBeginFunction; });

			if(m != n) //mamy funkcj� wewn�trzn�
			{
				++function_limit; //arbitralnie podnosimy limit funkcji

				o = std::find_if(it, m, IsChangingInstruction); //szukamy czy dziel� je jakies istotne instrukcje
				if( o == m || m - it == 1) //jest podejrzenie redefinicji, bo nie ma instrukcji zmieniaj�cych warto�� mi�dzy funkcjami (xx( lub pusto ((
				{
					MessageLog::Instance().AddMessage(MessageLog::ecFunctionRedefinitionInternal, m - instructions->begin() + 1);
				}
			}
			else //nie ma funkcji wewn�trznaj
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
	else if(it->operation == CodeTape::btoBeginLoop) // funkcja w p�tli
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
			MessageLog::Instance().AddMessage(MessageLog::ecCallButNoFunction, it - instructions->begin() + 1);	
		}
	}
	return false;
}

//Funkcja testuje sprawy zwi�zane z w�tkami
//1. Join, czy nie jest za wcze�nie wywo�any
//2. Czy kto� zapomnia�, �e fork zeruje bie��ca kom�rke i cos tam dodawa� odejmowa� wczesniej
//3 i 4 - powt�rzenia join i terminate
bool CodeAnalyser::TestForThreads(CodeIterator &it, const RepairFn2 & repairCB, const RepairFn2 & repairRepetitionCB)
{
	CodeIterator n, m;
	std::vector<CodeTape::bt_instruction>::reverse_iterator r;

	if(it->operation == CodeTape::btoJoin)
	{
		if(TestForRepetition(it, repairRepetitionCB))
			MessageLog::Instance().AddMessage(MessageLog::ecJoinRepeat, it - instructions->begin() + 1);
		
		
		if(IsWithinFunction(it) == false && forks == 0) //join poza funkcj�. Mo�e byc call do p�niejszej funkcji z fork, ale to trudno stwierdzi�
		{
			MessageLog::Instance().AddMessage(MessageLog::ecJoinBeforeFork, it - instructions->begin() + 1);	
			if(repairCB) //usuwamy join
			{
				repairCB(it, n);
			}
		}	
	}
	else if(it->operation == CodeTape::btoFork && it > instructions->begin()) 
	{
        r = std::vector<CodeTape::bt_instruction>::reverse_iterator(it);
		if(r != std::find_if_not(r, instructions->rend(), IsChangingCellInstruction))
		{
			MessageLog::Instance().AddMessage(MessageLog::ecRedundantOpBeforeFork, it - instructions->begin() + 1);	
		}
	}
	else if(it->operation == CodeTape::btoTerminate) 
	{
        if(TestForRepetition(it, repairRepetitionCB))
			MessageLog::Instance().AddMessage(MessageLog::ecTerminateRepeat, it - instructions->begin() + 1);
	}
	
	return false;
}

//Funkcja testuje sprawy zwi�zane z heap
//Szczegolnie switche [sa dystalne]
//1. Switch scope and repeat
//2. swap repeat
bool CodeAnalyser::TestForHeaps(CodeIterator &it, const RepairFn2 & repairCB, const RepairFn2 & repairRepetitionCB)
{
	CodeIterator n, m;
	std::vector<CodeTape::bt_instruction>::reverse_iterator r;

	if(it->operation == CodeTape::btoSwitchHeap)
	{
		if(TestForRepetition(it, repairRepetitionCB))
			MessageLog::Instance().AddMessage(MessageLog::ecSwitchRepeat, it - instructions->begin() + 1);

		n = std::find_if(it + 1, instructions->end(), IsSharedHeapInstruction);

		if(n == instructions->end()) 
		{
			MessageLog::Instance().AddMessage(MessageLog::ecRedundantSwitch, it - instructions->begin() + 1);

			if(repairCB) //usuwamy switch
			{
				repairCB(it, n);
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
        if(TestForRepetition(it, repairRepetitionCB))
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

//Funkcja testuje p�tle, czy:
//1. wszystkie grzecznie szybko d��� do zera
//2. Nie ma niepotrzebnych operacji + - przed
bool CodeAnalyser::TestLoopPerformance(CodeIterator &it)
{   
	CodeIterator n;
	int lim, s;

	if(it->operation == CodeTape::btoBeginLoop)
	{
		lim = GetLoopLimes(it);

		if(lim > 0) //wolna p�tla d���ca do niesko�czono�ci
		{
			MessageLog::Instance().AddMessage(MessageLog::ecSlowLoop, it - instructions->begin() + 1);	
		}

		//teraz czy przed p�tl� s� sensowne operacje
		if(it > instructions->begin()) //ofc tylko dla dalszych instrukcji
		{
			for(n = it - 1; n > instructions->begin(); --n)
			{
				if(IsArithmeticInstruction(*n) == false)
				   break; //kr�ci si� w ty�, a� znajdzie inna instrukcje + - albo pocz�tek ci�gu
			}

			if(it - 1 != n) //jest znaleziona przynajmniej jedna instrukcja + -
			{
				//tutaj n oznacza pierwsza nie + - instrukcje przed p�tl�
				s = Evaluate(n + 1, it);

				if(lim != 0 && (lim > 0 || (lim < 0 && s < 0) )) //je�eli p�tla jest policzalna, a sekwencja przed ni� da�y inaczej ni� p�tla
				{
					MessageLog::Instance().AddMessage(MessageLog::ecRedundantNearLoopArithmetic, it - instructions->begin() ); // specjalnie bez  + 1
				}
			}
		}
 
	}
	return false;
}

//Funkcja testuje operacje arytmetyczne, czy nie sa nadmiarowe
bool CodeAnalyser::TestArithmetics(CodeIterator &it, const RepairFn2i2 & repairCB)
{
	CodeIterator m, n;
	int sum, ops;

	if(IsArithmeticInstruction(*it))
	{
		ignore_arithmetic_test = true;
		n = std::find_if_not(it, instructions->end(), IsArithmeticInstruction);
		//mamy ci�g + - 
		//instrukcji musi byc wi�cej niz jedna i wynik ma byc osi�gni�ty najmniejsza liczba instrukcji, czyli bez np suma = 2 dla ++ [ops=2] a nie +-+-++ [ops=5]

		ops = std::count_if(it, n, IsArithmeticInstruction);
		sum = Evaluate(it, n);

		if((n - it) > 1 && abs(sum) != ops)
		{
			MessageLog::Instance().AddMessage(MessageLog::ecRedundantArithmetic, it - instructions->begin() + 1);	
			if(repairCB)
			{
				repairCB(it, n, sum, ops);
				
			}
		}
	}

	return false;
}

//Funkcja testuje operacje ruchu, czy nie sa nadmiarowe
bool CodeAnalyser::TestRedundantMoves(CodeIterator &it, const RepairFn2i2 & repairCB)
{
	CodeIterator n;
	int sum, ops;

	if(IsMoveInstruction(*it))
	{
		ignore_moves_test = true;
		n = std::find_if_not(it, instructions->end(), IsMoveInstruction);
		//mamy ci�g < > 
		//instrukcji musi byc wi�cej niz jedna i wynik ma byc osi�gni�ty najmniejsza liczba instrukcji, czyli bez np suma = 2 dla >> [ops=2] a nie <><>>> [ops=5]

		sum = EvaluateMoves(it, n);
		ops = std::count_if(it, n, IsMoveInstruction);
		
		if((n - it) > 1 && abs(sum) != ops)
		{
			MessageLog::Instance().AddMessage(MessageLog::ecRedundantMoves, it - instructions->begin() + 1);	
			if(repairCB)
			{
				repairCB(it, n, sum, ops);
			}
		}
	}

	return false;
}

//Funkcja testuje czy wystapi�y jakies powt�rzenia operator�w, np !
bool CodeAnalyser::TestForRepetition(CodeIterator &it, const RepairFn2 & repairCB)
{
	std::vector<CodeTape::bt_instruction>::iterator n;

	if (it + 1 < instructions->end())
	{
		n = std::find_if_not(it + 1, instructions->end(), [&it](const CodeTape::bt_instruction &o) { return o.operation == it->operation; });
		if (n != instructions->end()) //jest jakies powt�rzenie
		{
			if (repairCB) //usuwamy wszsytkie bez pierwszego
			{
				repairCB(it, n);
			}
			else it = n - 1;
		}
	}
	else return false;

	return true;
}

//Funkcja liczy sume wynikaj�ca z operacji + i - w pewnym ci�gu instrukcji od start do poprzedzaj�cego end
//Nie sa wa�ne inne operacje 
int CodeAnalyser::Evaluate(const CodeIterator &begin, const CodeIterator &end) const
{
	int sum = 0;
	for(CodeIterator it = begin; it < end; ++it)
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
//Nie sa wa�ne inne operacje 
int CodeAnalyser::EvaluateMoves(const CodeIterator &begin, const CodeIterator &end) const
{
	int sum = 0;
	for(CodeIterator it = begin; it < end; ++it)
	{
		switch(it->operation)
		{
			case CodeTape::btoMoveRight: ++sum; break;
			case CodeTape::btoMoveLeft:  --sum; break;
		}
	}

	return sum;
}

//Funkcja liczy spos�b zachowania si� p�tli
//Return 0 - niepoliczalne, -1: p�tla d��y do zera, 1: p�tla d��y do niesko�czono�ci
short CodeAnalyser::GetLoopLimes(const CodeIterator &op) const
{
	CodeIterator a, n, m;
	std::list<short> limesii; //wyniki podp�tli w kolejno�ci (to wa�ne jaka kolejno��)
	int s = 0;

	a = instructions->begin() + op->jump;
	m = op + 1;

	if(m == a)
		return 0; //niesko�czona p�tla

	n = std::find_if_not(m, a, [&op](const CodeTape::bt_instruction &o){ return IsArithmeticInstruction(o) || 
																				IsChangingInstruction(o) == false ||
																				o.operation == CodeTape::btoBeginLoop || 
																				o.operation == CodeTape::btoEndLoop; } );

	if(n == a) //s� tylko + - i p�tle i inne niewazne operacje
	{
		while(true)
		{
			n = std::find_if(m, a, [&op](const CodeTape::bt_instruction &o){ return o.operation == CodeTape::btoBeginLoop || 
																					 o.operation == CodeTape::btoEndLoop; }); //szukamy pierwszej instrukcji p�tli

			if(n != m) //sa jakies operacje -> [+-+-[
				limesii.push_back( Evaluate(m, a) < 0 ? -1 : 1 ); //p�tla z przewaga minus�w da tutaj -1, inaczej 1, zero liczy sie jak do nieksonczono�ci
			else   //nie ma �adnej operacji -> [[
				limesii.push_back( -1 ); //liczy sie jako normalna zdrowa operacje d���ca do zera

			if(n != a) //oho mamy p�tle w �rodku jak��, dawaj jeszcze jej limes
			{
				limesii.push_back(GetLoopLimes(n));
				m = instructions->begin() + n->jump + 1;
			}
			else break; //nie ma p�tli wewn�trznej
		}
		//mamy tutaj ci�g 1 i -1 , cos spr�bujemy policzy�

		for(std::list<short>::iterator it = limesii.begin(); it != limesii.end(); ++it)
		{
			if(*it == 0) //kt�rykolwiek element niepoliczalny wyklucza wszystko
				return 0;
		}

		return limesii.back() < 0 ? -1 : 1; //gdy ostatnie d��y w g�r� do nieskonczono�ci, na pewno si� co� zjebie 
	}
	
	return 0; // nie da si� policzy� limes
}


//Funkcja zwraca true, je�eli ��dana instrukcja znajduje si� w �rodku funkcji
//Liczymy poprostu nawiasy, inaczej nie b�dziemy pewni
bool CodeAnalyser::IsWithinFunction(const CodeIterator &op) const
{
	int opening_bracket_cnt, closing_bracket_cnt;

	opening_bracket_cnt = std::count_if(instructions->begin(), op, 
		                                [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoBeginFunction; });

	closing_bracket_cnt = std::count_if(instructions->begin(), op, 
		                                [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoEndFunction; });

	return opening_bracket_cnt > closing_bracket_cnt;
}

//Funkcje modyfikuj� linkowania 
void CodeAnalyser::RelinkCommands(const CodeIterator &start, short n)
{
	RelinkCommands(start, instructions->end(), n);
}

void CodeAnalyser::RelinkCommands(const CodeIterator &start, const CodeIterator &end, short n)
{
	for(CodeIterator it = start; it < end; ++it)
	{
		if(IsLinkedInstruction(*it) || it->NullJump() == false)
		{
			it->jump -= n;
		}
	}
}

//Testuje poprawno�� linkujacych komend - ka�da z nich musi linkowa� do innej, nie mog� byc bez par
bool CodeAnalyser::TestLinks()
{
	for(CodeIterator it = instructions->begin(); it < instructions->end(); ++it)
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

/*zapasowa dzialaj�ca dla [+-] bez innych instrukcji
short CodeAnalyser::GetLoopLimes(const std::vector<CodeTape::bt_instruction>::iterator &op) const
{
	std::vector<CodeTape::bt_instruction>::iterator a, n, m;
	std::list<short> limesii; //wyniki podp�tli w kolejno�ci (to wa�ne jaka kolejno��)
	int s = 0;

	a = instructions->begin() + op->jump;
	m = op + 1;

	if(m == a)
		return 0; //niesko�czona p�tla

	n = std::find_if_not(m, a, [&op](const CodeTape::bt_instruction &o){ return IsArithmeticInstruction(o) ||
																					 o.operation == CodeTape::btoBeginLoop || 
																					 o.operation == CodeTape::btoEndLoop; } );

	if(n == a) //s� tylko + - i p�tle
	{
		while(true)
		{
			n = std::find_if_not(m, a, IsArithmeticInstruction); //szukamy pierwszej instrukcji nie + -, czyli p�tli

			if(n != m) //sa jakies operacje -> [+-+-[
				limesii.push_back( Calcule(m, a) < 0 ? -1 : 1 ); //p�tla z przewaga minus�w da tutaj -1, inaczej 1, zero liczy sie jak do nieksonczono�ci
			else   //nie ma �adnej operacji -> [[
				limesii.push_back( -1 ); //liczy sie jako normalna zdrowa operacje d���ca do zera

			if(n != a) //oho mamy p�tle w �rodku jak��, dawaj jeszcze jej limes
			{
				limesii.push_back(GetLoopLimes(n));
				m = instructions->begin() + n->jump + 1;
			}
			else break; //nie ma p�tli wewn�trznej
		}
		//mamy tutaj ci�g 1 i -1 , cos spr�bujemy policzy�

		for(std::list<short>::iterator it = limesii.begin(); it != limesii.end(); ++it)
		{
			if(*it == 0) //kt�rykolwiek element niepoliczalny wyklucza wszystko
				return 0;
		}

		return limesii.back() < 0 ? -1 : 1; //gdy ostatnie d��y w g�r� do nieskonczono�ci, na pewno si� co� zjebie 
	}
	
	return 0; // nie da si� policzy� limes
}
*/
