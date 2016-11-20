#include "Debuger.h"

#include <algorithm>
#include <cmath>
#include <list>
//#include <iostream>

Debuger::Debuger(std::vector<CodeTape::bt_instruction> *instructions, bool repair)
{
	language = Parser::clBrainThread;
	
	if(instructions == nullptr)
		throw std::invalid_argument("Debuger::Debuger: invalid argument");

	this->instructions = instructions;
	this->repair = repair;
	this->typesize = 1;

	repaired_issues = 0;

	function_limit = static_cast<unsigned int>(std::pow(2.0, typesize*8));
}

Debuger::~Debuger(void)
{
}

//Operatory arytmetyczne: + -
bool Debuger::IsArithmeticInstruction(const CodeTape::bt_instruction &op)
{
	return (op.operation == CodeTape::btoIncrement ||
		    op.operation == CodeTape::btoDecrement ); 
}

//Operatory ruchu pi�rka: < >
bool Debuger::IsMoveInstruction(const CodeTape::bt_instruction &op)
{
	return (op.operation == CodeTape::btoMoveLeft ||
		   op.operation == CodeTape::btoMoveRight); 
}

//Operatory zmieniaj�ce przep�yw lub warto�� kom�rek
//Wszystkie operatory Brainfuck (bez output) + function Call
//Nie ma tutaj definicji funkcji! ()
bool Debuger::IsChangingInstruction(const CodeTape::bt_instruction &op)
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
bool Debuger::IsLinkedInstruction(const CodeTape::bt_instruction &op)
{
	return (op.operation == CodeTape::btoBeginLoop	   ||
		    op.operation == CodeTape::btoEndLoop	   ||
		    op.operation == CodeTape::btoBeginFunction ||
		    op.operation == CodeTape::btoEndFunction); 
}

//Operatory dla testu Przed forkiem
bool Debuger::IsChangingCellInstruction(const CodeTape::bt_instruction &op)
{
	return (IsArithmeticInstruction(op) ||
		    op.operation == CodeTape::btoPop || 
			op.operation == CodeTape::btoSharedPop ||
			op.operation == CodeTape::btoAsciiRead || 
			op.operation == CodeTape::btoDecimalRead);
}


void Debuger::Debug(void)
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

		if(TestThreads(it))
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
		MessageLog::GetInstance().AddMessage(MessageLog::ecJoinButNoFork, 1);
	}

	if( function_def > function_limit)
	{
		MessageLog::GetInstance().AddMessage(MessageLog::ecFunctionLimitExceed, 1);
	}

	if( function_def > 0 && function_calls == 0)
	{
		MessageLog::GetInstance().AddMessage(MessageLog::ecFunctionExistsButNoCall, 1);
	}

	if(TestLinks())
	{
		MessageLog::GetInstance().AddMessage(MessageLog::ecIntegrityLost, 1);
	}

	/*std::cout << std::endl;
	int q=0;
	for(std::vector<CodeTape::bt_instruction>::iterator it = instructions->begin(); it < instructions->end(); ++it)
	{
		std::cout << q++ << ": " << it->operation << ": " << (it->NullJump() ? 115 : it->jump) << "\n";
	}
	std::cout << std::endl;*/
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
bool Debuger::TestForInfiniteLoops(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	if(it->operation == CodeTape::btoBeginLoop)
	{
		//niesko�czona p�tla []
		if( (it + 1)->operation == CodeTape::btoEndLoop)
		{ 
			MessageLog::GetInstance().AddMessage(MessageLog::ecInfiniteLoop, it - instructions->begin() + 1);	
			if(repair)
			{
				it = instructions->erase(it, it+2);
				this->RelinkCommands(it, 2);
				++repaired_issues;
				return true;
			}
		}
		//pusta p�tla [[xxxx]]
		else if( (it + 1)->operation == CodeTape::btoBeginLoop && (instructions->begin() + (it->jump) - 1)->operation == CodeTape::btoEndLoop)
		{
			MessageLog::GetInstance().AddMessage(MessageLog::ecEmptyLoop, it - instructions->begin() + 1);
			if(repair)
			{
				instructions->erase(instructions->begin() + (it->jump));
				it = instructions->erase(it);
				this->RelinkCommands(it, instructions->begin() + (it->jump), 1); //�wiadomie uzywam starej pozycji
				this->RelinkCommands(instructions->begin() + (it->jump)+1, 2); //od ko�ca usuni�tej p�tli juz po dwa
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
//4. Funkcja w p�tli 
bool Debuger::TestForFunctionsErrors(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	std::vector<CodeTape::bt_instruction>::iterator m, n, o;

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
				MessageLog::GetInstance().AddMessage(MessageLog::ecFunctionRedefinition, n - instructions->begin() + 1);
			}
		}
	}
	else if(it->operation == CodeTape::btoBeginFunction) 
	{
		n = instructions->begin() + it->jump;//bierzemy ca�� tre�� funkcji

		//pusta funkcja  ()
		if((it + 1)->operation == CodeTape::btoEndFunction) 
		{
			MessageLog::GetInstance().AddMessage(MessageLog::ecEmptyFunction, it - instructions->begin() + 1);	
			if(repair) //usuwamy funkcj�
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
			MessageLog::GetInstance().AddMessage(MessageLog::ecEmptyFunction, it - instructions->begin() + 1);
			if(repair)
			{
				instructions->erase(instructions->begin() + (it->jump));
				it = instructions->erase(it);
				this->RelinkCommands(it, instructions->begin() + (it->jump), 1); //�wiadomie uzywam starej pozycji
				this->RelinkCommands(instructions->begin() + (it->jump)+1, 2); //od ko�ca usuni�tej p�tli juz po dwa
				++repaired_issues;
				return true;
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
					MessageLog::GetInstance().AddMessage(MessageLog::ecFunctionRedefinitionInternal, m - instructions->begin() + 1);
				}
			}
			else //nie ma funkcji wewn�trznaj
			{
				m = std::find_if(it + 1, n, 
					             [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoCallFunction; });

				o = std::find_if(it + 1, m, IsChangingInstruction); //szukamy zmieniajacych instrukcji do momentu pierwszego call, nastpne calle niewazne
				if( m != n && o == m)  //jest call i nie ma instrukcji zmieniajacych 
			    {
					MessageLog::GetInstance().AddMessage(MessageLog::ecInfinityRecurention, m - instructions->begin() + 1);
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
			MessageLog::GetInstance().AddMessage(MessageLog::ecFunctionInLoop, m - instructions->begin() + 1);
		}
	}
	return false;
}

//Funkcja testuje sprawy zwi�zane z w�tkami
//1. Join, czy nie jest za wcze�nie wywo�any
//2. Czy kto� zapomnia�, �e fork zeruje bie��ca kom�rke i cos tam dodawa� odejmowa� wczesniej
//3 i 4 - powt�rzenia join i terminate
//5 i 6  - powt�rzenia swap�w
bool Debuger::TestThreads(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	std::vector<CodeTape::bt_instruction>::iterator n, m;
	std::vector<CodeTape::bt_instruction>::reverse_iterator r;

	if(it->operation == CodeTape::btoJoin)
	{
		if(TestForRepetition(it, CodeTape::btoJoin))
			MessageLog::GetInstance().AddMessage(MessageLog::ecJoinRepeat, it - instructions->begin() + 1);
		
		
		if(IsWithinFunction(it) == false && forks == 0) //join poza funkcj�. Mo�e byc call do p�niejszej funkcji z fork, ale to trudno stwierdzi�
		{
			MessageLog::GetInstance().AddMessage(MessageLog::ecJoinBeforeFork, it - instructions->begin() + 1);	
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
			MessageLog::GetInstance().AddMessage(MessageLog::ecRedundantOpBeforeFork, (r+1).base() - instructions->begin());	
		}
	}
	else if(it->operation == CodeTape::btoTerminate) 
	{
        if(TestForRepetition(it, CodeTape::btoTerminate))
			MessageLog::GetInstance().AddMessage(MessageLog::ecTerminateRepeat, it - instructions->begin() + 1);
	}
	else if(it->operation == CodeTape::btoSwap || it->operation == CodeTape::btoSharedSwap) 
	{
        if(TestForRepetition(it, CodeTape::btoSwap) || TestForRepetition(it, CodeTape::btoSharedSwap))
			MessageLog::GetInstance().AddMessage(MessageLog::ecSwapRepeat, it - instructions->begin() + 1);
	}
	return false;
}

//Funkcja testuje p�tle, czy:
//1. wszystkie grzecznie szybko d��� do zera
//2. Nie ma niepotrzebnych operacji + - przed
bool Debuger::TestLoopPerformance(std::vector<CodeTape::bt_instruction>::iterator &it)
{   
	std::vector<CodeTape::bt_instruction>::iterator n;
	int lim, s;

	if(it->operation == CodeTape::btoBeginLoop)
	{
		lim = GetLoopLimes(it);

		if(lim > 0) //wolna p�tla d���ca do niesko�czono�ci
		{
			MessageLog::GetInstance().AddMessage(MessageLog::ecSlowLoop, it - instructions->begin() + 1);	
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
				s = Calcule(n + 1, it);

				if(lim != 0 && (lim > 0 || (lim < 0 && s < 0) )) //je�eli p�tla jest policzalna, a sekwencja przed ni� da�y inaczej ni� p�tla
				{
					MessageLog::GetInstance().AddMessage(MessageLog::ecRedundantNearLoopArithmetic, it - instructions->begin() ); // specjalnie bez  + 1
				}
			}
		}
 
	}
	return false;
}

//Funkcja testuje operacje arytmetyczne, czy nie sa nadmiarowe
bool Debuger::TestArithmetics(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	std::vector<CodeTape::bt_instruction>::iterator m, n;
	int sum, ops, k;

	if(IsArithmeticInstruction(*it))
	{
		ignore_arithmetic_test = true;
		n = std::find_if_not(it, instructions->end(), IsArithmeticInstruction);
		//mamy ci�g + - 
		//instrukcji musi byc wi�cej niz jedna i wynik ma byc osi�gni�ty najmniejsza liczba instrukcji, czyli bez np suma = 2 dla ++ [ops=2] a nie +-+-++ [ops=5]

		ops = std::count_if(it, n, IsArithmeticInstruction);
		sum = Calcule(it, n);

		if((n - it) > 1 && abs(sum) != ops)
		{
			MessageLog::GetInstance().AddMessage(MessageLog::ecRedundantArithmetic, it - instructions->begin() + 1);	
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
bool Debuger::TestRedundantMoves(std::vector<CodeTape::bt_instruction>::iterator &it)
{
	std::vector<CodeTape::bt_instruction>::iterator n;
	int sum, k, ops;

	if(IsMoveInstruction(*it))
	{
		ignore_moves_test = true;
		n = std::find_if_not(it, instructions->end(), IsMoveInstruction);
		//mamy ci�g < > 
		//instrukcji musi byc wi�cej niz jedna i wynik ma byc osi�gni�ty najmniejsza liczba instrukcji, czyli bez np suma = 2 dla >> [ops=2] a nie <><>>> [ops=5]

		sum = CalculeMoves(it, n);
		ops = std::count_if(it, n, IsMoveInstruction);
		
		if((n - it) > 1 && abs(sum) != ops)
		{
			MessageLog::GetInstance().AddMessage(MessageLog::ecRedundantMoves, it - instructions->begin() + 1);	
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

//Funkcja testuje czy wystapi�y jakies powt�rzenia operator�w, np !
bool Debuger::TestForRepetition(std::vector<CodeTape::bt_instruction>::iterator &it, const CodeTape::bt_operation &op)
{
	std::vector<CodeTape::bt_instruction>::iterator n;

	if(it->operation == op && it + 1 < instructions->end() && (it+1)->operation == op)
	{
		n = std::find_if_not(it + 1, instructions->end(), [&op](const CodeTape::bt_instruction &o){ return o.operation == op; });
		if(n != instructions->end()) //jest jakies powt�rzenie
		{
			if(repair) //usuwamy wszsytkie bez pierwszego
			{
				this->RelinkCommands(it, (n-it-1));
				it = instructions->erase(it+1, n);
				++repaired_issues;
			}
			else it = n;
		}
	}
	else return false;

	return true;
}

//Funkcja liczy sume wynikaj�ca z operacji + i - w pewnym ci�gu instrukcji od start do poprzedzaj�cego end
//Nie sa wa�ne inne operacje 
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

//Funkcja liczy operacje < i >, cel taki jak w Calcule
//Nie sa wa�ne inne operacje 
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

//Funkcja liczy spos�b zachowania si� p�tli
//Return 0 - niepoliczalne, -1: p�tla d��y do zera, 1: p�tla d��y do niesko�czono�ci
short Debuger::GetLoopLimes(const std::vector<CodeTape::bt_instruction>::iterator &op) const
{
	std::vector<CodeTape::bt_instruction>::iterator a, n, m;
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


//Funkcja zwraca true, je�eli ��dana instrukcja znajduje si� w �rodku funkcji
//Liczymy poprostu nawiasy, inaczej nie b�dziemy pewni
bool Debuger::IsWithinFunction(const std::vector<CodeTape::bt_instruction>::iterator &op) const
{
	int opening_bracket_cnt, closing_bracket_cnt;

	opening_bracket_cnt = std::count_if(instructions->begin(), op, 
		                                [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoBeginFunction; });

	closing_bracket_cnt = std::count_if(instructions->begin(), op, 
		                                [](const CodeTape::bt_instruction &op){ return op.operation == CodeTape::btoEndFunction; });

	return opening_bracket_cnt > closing_bracket_cnt;
}

//Funkcje modyfikuj� linkowania 
void Debuger::RelinkCommands(const std::vector<CodeTape::bt_instruction>::iterator &start, short n)
{
	RelinkCommands(start, instructions->end(), n);
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

//Testuje poprawno�� linkujacych komend - ka�da z nich musi linkowa� do innej, nie mog� byc bez par
bool Debuger::TestLinks()
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

bool Debuger::isCodeValid(void)
{
	return MessageLog::GetInstance().WarningsCount() == 0;
}

/*zapasowa dzialaj�ca dla [+-] bez innych instrukcji
short Debuger::GetLoopLimes(const std::vector<CodeTape::bt_instruction>::iterator &op) const
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
