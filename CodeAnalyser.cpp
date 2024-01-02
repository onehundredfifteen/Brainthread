#include "CodeAnalyser.h"

#include <algorithm>
#include <cmath>
#include <list>

namespace BT {

	CodeAnalyser::CodeAnalyser(std::vector<bt_instruction>& _instructions)
		: language(CodeLang::clBrainThread), typesize(1), instructions(_instructions)
	{
		repaired_issues = 0;

		function_limit = static_cast<unsigned int>(std::pow(2.0, typesize * 8));
	}

	CodeAnalyser::~CodeAnalyser(void)
	{
	}

	//Operatory arytmetyczne: + -
	bool CodeAnalyser::IsArithmeticInstruction(const bt_instruction& ins)
	{
		return (ins.operation == bt_operation::btoIncrement ||
			ins.operation == bt_operation::btoDecrement);
	}

	//Operatory ruchu piórka: < >
	bool CodeAnalyser::IsMoveInstruction(const bt_instruction& ins)
	{
		return (ins.operation == bt_operation::btoMoveLeft ||
			ins.operation == bt_operation::btoMoveRight);
	}

	//Operatory zmieniaj¹ce przep³yw lub wartoœæ komórek
	//Wszystkie operatory Brainfuck (bez output) + function Call
	//Nie ma tutaj definicji funkcji! ()
	bool CodeAnalyser::IsChangingInstruction(const bt_instruction& ins)
	{
		return (IsArithmeticInstruction(ins) ||
			IsMoveInstruction(ins) ||
			ins.operation == bt_operation::btoAsciiRead ||
			ins.operation == bt_operation::btoDecimalRead ||
			ins.operation == bt_operation::btoBeginLoop ||
			ins.operation == bt_operation::btoEndLoop ||
			ins.operation == bt_operation::btoCallFunction ||
			ins.operation == bt_operation::btoPop ||
			ins.operation == bt_operation::btoSharedPop);
	}

	//Operatory ³¹czone parami z innymi: pêtle i funkcje
	bool CodeAnalyser::IsLinkedInstruction(const bt_instruction& ins)
	{
		return (ins.operation == bt_operation::btoBeginLoop ||
			ins.operation == bt_operation::btoEndLoop ||
			ins.operation == bt_operation::btoBeginFunction ||
			ins.operation == bt_operation::btoEndFunction);
	}

	//Operatory dla testu Przed forkiem
	bool CodeAnalyser::IsChangingCellInstruction(const bt_instruction& ins)
	{
		return (IsArithmeticInstruction(ins) ||
			ins.operation == bt_operation::btoPop ||
			ins.operation == bt_operation::btoSharedPop ||
			ins.operation == bt_operation::btoAsciiRead ||
			ins.operation == bt_operation::btoDecimalRead);
	}

	//Operatory dla testu Przed forkiem
	bool CodeAnalyser::IsSharedHeapInstruction(const bt_instruction& ins)
	{
		return (ins.operation == bt_operation::btoSharedPop ||
			ins.operation == bt_operation::btoSharedPush ||
			ins.operation == bt_operation::btoSharedSwap);
	}

	//Operatory ³¹czone parami z innymi: pêtle i funkcje
	bool CodeAnalyser::IsFlowChangingInstruction(const bt_instruction& ins)
	{
		return (IsLinkedInstruction(ins) ||
			ins.operation == bt_operation::btoCallFunction);
	}


	void CodeAnalyser::Analyse(void)
	{
		CodeTapeIterator it = instructions.begin();

		function_def = 0;
		function_calls = 0;
		forks = 0;
		joins = 0;
		ignore_arithmetic_test = false;
		ignore_moves_test = false;

		while (it < instructions.end())
		{
			//reset flag
			if (IsArithmeticInstruction(*it) == false && ignore_arithmetic_test == true)
				ignore_arithmetic_test = false;

			if (IsMoveInstruction(*it) == false && ignore_moves_test == true)
				ignore_moves_test = false;

			//testy

			if (TestForInfiniteLoops(it))
				continue;

			if (TestForFunctionsErrors(it))
				continue;

			if (TestForThreads(it))
				continue;

			if (TestForHeaps(it))
				continue;

			if (TestLoopPerformance(it))
				continue;

			if (ignore_arithmetic_test == false)
			{
				if (TestArithmetics(it))
					continue;
			}

			if (ignore_moves_test == false)
			{
				if (TestRedundantMoves(it))
					continue;
			}

			if (it->operation == bt_operation::btoFork)
				++forks;
			else if (it->operation == bt_operation::btoJoin)
				++joins;
			else if (it->operation == bt_operation::btoCallFunction)
				++function_calls;

			++it;
		}

		//dodatkowe b³êdy statystyczne
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

		/*std::cout << std::endl;
		int q=0;
		for(std::vector<bt_operation::bt_instruction>::iterator it = instructions.begin(); it < instructions.end(); ++it)
		{
			std::cout << q++ << ": " << it->operation << ": " << (it->NullJump() ? 115 : it->jump) << "\n";
		}
		std::cout << std::endl;*/
	}

	void CodeAnalyser::Repair(void)
	{
		CodeTapeIterator it = instructions.begin();

		function_def = 0;
		function_calls = 0;
		forks = 0;
		joins = 0;
		ignore_arithmetic_test = false;
		ignore_moves_test = false;

		//funkcje naprawcze
		auto repairRepetition = [this](CodeTapeIterator& _it, CodeTapeIterator& n) {
			this->RelinkCommands(_it, (n - _it - 1));
			_it = instructions.erase(_it + 1, n);
			++repaired_issues;
			return false;
		};

		auto repairH = [this](CodeTapeIterator& _it, CodeTapeIterator& n) {
			_it = instructions.erase(_it);
			this->RelinkCommands(_it, (n - _it - 1));
			++repaired_issues;
			return true;
		};

		while (it < instructions.end())
		{
			//reset flag
			if (IsArithmeticInstruction(*it) == false && ignore_arithmetic_test == true)
				ignore_arithmetic_test = false;

			if (IsMoveInstruction(*it) == false && ignore_moves_test == true)
				ignore_moves_test = false;

			//testy



			if (TestForInfiniteLoops(it,
				[this](CodeTapeIterator& _it) {
				_it = instructions.erase(_it, _it + 2);
				RelinkCommands(_it, 2);
				++repaired_issues;
			},
				[this](CodeTapeIterator& _it) {
				instructions.erase(instructions.begin() + (_it->jump));
				_it = instructions.erase(_it);
				RelinkCommands(_it, instructions.begin() + (_it->jump), 1); //œwiadomie uzywam starej pozycji
				RelinkCommands(instructions.begin() + (_it->jump) + 1, 2); //od koñca usuniêtej pêtli juz po dwa
				++repaired_issues;
			}))
				continue;

			if (TestForFunctionsErrors(it,
				[this](CodeTapeIterator& _it, CodeTapeIterator& n) {
				instructions.erase(n);
				_it = instructions.erase(_it);
				this->RelinkCommands(_it, 2);
				++repaired_issues;
			},
				[this](CodeTapeIterator& _it) {
				instructions.erase(instructions.begin() + (_it->jump));
				_it = instructions.erase(_it);
				this->RelinkCommands(_it, instructions.begin() + (_it->jump), 1); //œwiadomie uzywam starej pozycji
				this->RelinkCommands(instructions.begin() + (_it->jump) + 1, 2); //od koñca usuniêtej pêtli juz po dwa
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
					[this](CodeTapeIterator& _it, CodeTapeIterator& n, int& sum, int& ops) {
					int k = (ops - abs(sum)) / 2;

					for (int i = 0; i < k; ++i)
					{
						std::find_if(_it, n, [](const bt_instruction& op) { return op.operation == bt_operation::btoDecrement; })->operation = bt_operation::btoUnkown;
						std::find_if(_it, n, [](const bt_instruction& op) { return op.operation == bt_operation::btoIncrement; })->operation = bt_operation::btoUnkown;
					}

					instructions.erase(
						std::remove_if(_it, instructions.end(), [](const bt_instruction& op) { return op.operation == bt_operation::btoUnkown; }),
						instructions.end()),
						this->RelinkCommands(_it, (ops - sum));

					++repaired_issues;
				}))
					continue;
			}

			if (ignore_moves_test == false)
			{
				if (TestRedundantMoves(it,
					[this](CodeTapeIterator& _it, CodeTapeIterator& n, int& sum, int& ops) {
					int k = (ops - abs(sum)) / 2;
					for (int i = 0; i < k; ++i)
					{
						std::find_if(_it, n, [](const bt_instruction& op) { return op.operation == bt_operation::btoMoveLeft; })->operation = bt_operation::btoUnkown;
						std::find_if(_it, n, [](const bt_instruction& op) { return op.operation == bt_operation::btoMoveRight; })->operation = bt_operation::btoUnkown;
					}

					instructions.erase(
						std::remove_if(_it, instructions.end(), [](const bt_instruction& op) { return op.operation == bt_operation::btoUnkown; }),
						instructions.end()),
						this->RelinkCommands(_it, (ops - sum));
					++repaired_issues;
				}))
					continue;
			}

			if (it->operation == bt_operation::btoFork)
				++forks;
			else if (it->operation == bt_operation::btoJoin)
				++joins;
			else if (it->operation == bt_operation::btoCallFunction)
				++function_calls;

			++it;
		}

		//dodatkowe b³êdy statystyczne
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
	bool CodeAnalyser::TestForInfiniteLoops(CodeTapeIterator& it, const RepairFn& infLoopRep, const RepairFn& emptyLoopRep)
	{
		if (it->operation == bt_operation::btoBeginLoop)
		{
			//nieskoñczona pêtla []
			if ((it + 1)->operation == bt_operation::btoEndLoop)
			{
				MessageLog::Instance().AddMessage(MessageLog::ecInfiniteLoop, it - instructions.begin() + 1);
				if (infLoopRep)
				{
					infLoopRep(it);
				}
			}
			//pusta pêtla [[xxxx]]
			else if ((it + 1)->operation == bt_operation::btoBeginLoop && (instructions.begin() + (it->jump) - 1)->operation == bt_operation::btoEndLoop)
			{
				MessageLog::Instance().AddMessage(MessageLog::ecEmptyLoop, it - instructions.begin() + 1);
				if (emptyLoopRep)
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
	//4. Funkcja w pêtli 
	bool CodeAnalyser::TestForFunctionsErrors(CodeTapeIterator& it, const RepairFn2& emptyFunType1Rep, const RepairFn& emptyFunType2Rep)
	{
		CodeTapeIterator m, n, o;
		std::vector<bt_instruction>::reverse_iterator r, s;

		if (it->operation == bt_operation::btoEndFunction)
		{
			//szukam nastêpnej funkcji
			n = std::find_if(it, instructions.end(),
				[](const bt_instruction& op) { return op.operation == bt_operation::btoBeginFunction; });

			if (n != instructions.end()) //jest - sprawdzamy instrukcje pomiedzy ) i (
			{
				m = std::find_if(it, n, IsChangingInstruction);
				if (m == n || n - it == 1) //nie ma zmieniajacych coœ instrukcji lub w ogóle nie ma nic
				{
					//jest podejrzenie redefinicji, bo nie ma instrukcji zmieniaj¹cych wartoœæ miêdzy funkcjami
					MessageLog::Instance().AddMessage(MessageLog::ecFunctionRedefinition, n - instructions.begin() + 1);
				}
			}
		}
		else if (it->operation == bt_operation::btoBeginFunction)
		{
			n = instructions.begin() + it->jump;//bierzemy ca³¹ treœæ funkcji

			//pusta funkcja  ()
			if ((it + 1)->operation == bt_operation::btoEndFunction)
			{
				MessageLog::Instance().AddMessage(MessageLog::ecEmptyFunction, it - instructions.begin() + 1);
				if (emptyFunType1Rep) //usuwamy funkcjê
				{
					emptyFunType1Rep(it, n);
				}
			}
			//pusta funkcja ((xx))
			else if ((it + 1)->operation == bt_operation::btoBeginFunction && (instructions.begin() + (it->jump) - 1)->operation == bt_operation::btoEndFunction)
			{
				MessageLog::Instance().AddMessage(MessageLog::ecEmptyFunction, it - instructions.begin() + 1);
				if (emptyFunType2Rep)
				{
					emptyFunType2Rep(it);
				}
			}
			else // nie jest pusta -> szukamy funkcji wewêtrznej (bez pierwszego elementu, bo to wlasnie '(' )
			{
				m = std::find_if(it + 1, n,
					[](const bt_instruction& op) { return op.operation == bt_operation::btoBeginFunction; });

				if (m != n) //mamy funkcjê wewnêtrzn¹
				{
					++function_limit; //arbitralnie podnosimy limit funkcji

					o = std::find_if(it, m, IsChangingInstruction); //szukamy czy dziel¹ je jakies istotne instrukcje
					if (o == m || m - it == 1) //jest podejrzenie redefinicji, bo nie ma instrukcji zmieniaj¹cych wartoœæ miêdzy funkcjami (xx( lub pusto ((
					{
						MessageLog::Instance().AddMessage(MessageLog::ecFunctionRedefinitionInternal, m - instructions.begin() + 1);
					}
				}
				else //nie ma funkcji wewnêtrznaj
				{
					m = std::find_if(it + 1, n,
						[](const bt_instruction& op) { return op.operation == bt_operation::btoCallFunction; });

					o = std::find_if(it + 1, m, IsChangingInstruction); //szukamy zmieniajacych instrukcji do momentu pierwszego call, nastpne calle niewazne
					if (m != n && o == m)  //jest call i nie ma instrukcji zmieniajacych 
					{
						MessageLog::Instance().AddMessage(MessageLog::ecInfinityRecurention, m - instructions.begin() + 1);
					}
				}

			}

			++function_def;
		}
		else if (it->operation == bt_operation::btoBeginLoop) // funkcja w pêtli
		{
			n = instructions.begin() + it->jump;

			m = std::find_if(it + 1, n,
				[](const bt_instruction& op) { return op.operation == bt_operation::btoBeginFunction; });

			if (m != n)
			{
				MessageLog::Instance().AddMessage(MessageLog::ecFunctionInLoop, m - instructions.begin() + 1);
			}
		}
		else if (it->operation == bt_operation::btoCallFunction) //undefined call
		{
			r = std::vector<bt_instruction>::reverse_iterator(it);
			s = std::find_if(r, instructions.rend(),
				[](const bt_instruction& op) { return op.operation == bt_operation::btoBeginFunction; });

			if (s == instructions.rend())
			{
				MessageLog::Instance().AddMessage(MessageLog::ecCallButNoFunction, it - instructions.begin() + 1);
			}
		}
		return false;
	}

	//Funkcja testuje sprawy zwi¹zane z w¹tkami
	//1. Join, czy nie jest za wczeœnie wywo³any
	//2. Czy ktoœ zapomnia³, ¿e fork zeruje bie¿¹ca komórke i cos tam dodawa³ odejmowa³ wczesniej
	//3 i 4 - powtórzenia join i terminate
	bool CodeAnalyser::TestForThreads(CodeTapeIterator& it, const RepairFn2& repairCB, const RepairFn2& repairRepetitionCB)
	{
		CodeTapeIterator n, m;
		std::vector<bt_instruction>::reverse_iterator r;

		if (it->operation == bt_operation::btoJoin)
		{
			if (TestForRepetition(it, repairRepetitionCB))
				MessageLog::Instance().AddMessage(MessageLog::ecJoinRepeat, it - instructions.begin() + 1);


			if (IsWithinFunction(it) == false && forks == 0) //join poza funkcj¹. Mo¿e byc call do póŸniejszej funkcji z fork, ale to trudno stwierdziæ
			{
				MessageLog::Instance().AddMessage(MessageLog::ecJoinBeforeFork, it - instructions.begin() + 1);
				if (repairCB) //usuwamy join
				{
					repairCB(it, n);
				}
			}
		}
		else if (it->operation == bt_operation::btoFork && it > instructions.begin())
		{
			r = std::vector<bt_instruction>::reverse_iterator(it);
			if (r != std::find_if_not(r, instructions.rend(), IsChangingCellInstruction))
			{
				MessageLog::Instance().AddMessage(MessageLog::ecRedundantOpBeforeFork, it - instructions.begin() + 1);
			}
		}
		else if (it->operation == bt_operation::btoTerminate)
		{
			if (TestForRepetition(it, repairRepetitionCB))
				MessageLog::Instance().AddMessage(MessageLog::ecTerminateRepeat, it - instructions.begin() + 1);
		}

		return false;
	}

	//Funkcja testuje sprawy zwi¹zane z heap
	//Szczegolnie switche [sa dystalne]
	//1. Switch scope and repeat
	//2. swap repeat
	bool CodeAnalyser::TestForHeaps(CodeTapeIterator& it, const RepairFn2& repairCB, const RepairFn2& repairRepetitionCB)
	{
		CodeTapeIterator n, m;
		std::vector<bt_instruction>::reverse_iterator r;

		if (it->operation == bt_operation::btoSwitchHeap)
		{
			if (TestForRepetition(it, repairRepetitionCB))
				MessageLog::Instance().AddMessage(MessageLog::ecSwitchRepeat, it - instructions.begin() + 1);

			n = std::find_if(it + 1, instructions.end(), IsSharedHeapInstruction);

			if (n == instructions.end())
			{
				MessageLog::Instance().AddMessage(MessageLog::ecRedundantSwitch, it - instructions.begin() + 1);

				if (repairCB) //usuwamy switch
				{
					repairCB(it, n);
				}
			}
			else // jest jakas instrukcja shared heapa
			{
				m = std::find_if(it + 1, n, IsFlowChangingInstruction);

				if (m != n)
					MessageLog::Instance().AddMessage(MessageLog::ecSwithOutOfScope, it - instructions.begin() + 1);
			}
		}
		else if (it->operation == bt_operation::btoSwap)
		{
			if (TestForRepetition(it, repairRepetitionCB))
				MessageLog::Instance().AddMessage(MessageLog::ecSwapRepeat, it - instructions.begin() + 1);
		}
		else if (it->operation == bt_operation::btoSharedSwap)
		{   //we have ~ op beetween
			n = std::find_if(it + 1, instructions.end(),
				[](const bt_instruction& op) { return op.operation == bt_operation::btoSharedSwap; });

			if (n != instructions.end() && n - it == 2) //~%~%
			{
				MessageLog::Instance().AddMessage(MessageLog::ecSwapRepeat, it - instructions.begin() + 1);
			}
		}
		return false;
	}

	//Funkcja testuje pêtle, czy:
	//1. wszystkie grzecznie szybko d¹¿¹ do zera
	//2. Nie ma niepotrzebnych operacji + - przed
	bool CodeAnalyser::TestLoopPerformance(CodeTapeIterator& it)
	{
		CodeTapeIterator n;
		int lim, s;

		if (it->operation == bt_operation::btoBeginLoop)
		{
			lim = GetLoopLimes(it);

			if (lim > 0) //wolna pêtla d¹¿¹ca do nieskoñczonoœci
			{
				MessageLog::Instance().AddMessage(MessageLog::ecSlowLoop, it - instructions.begin() + 1);
			}

			//teraz czy przed pêtl¹ s¹ sensowne operacje
			if (it > instructions.begin()) //ofc tylko dla dalszych instrukcji
			{
				for (n = it - 1; n > instructions.begin(); --n)
				{
					if (IsArithmeticInstruction(*n) == false)
						break; //krêci siê w ty³, a¿ znajdzie inna instrukcje + - albo pocz¹tek ci¹gu
				}

				if (it - 1 != n) //jest znaleziona przynajmniej jedna instrukcja + -
				{
					//tutaj n oznacza pierwsza nie + - instrukcje przed pêtl¹
					s = Evaluate(n + 1, it);

					if (lim != 0 && (lim > 0 || (lim < 0 && s < 0))) //je¿eli pêtla jest policzalna, a sekwencja przed ni¹ da¿y inaczej ni¿ pêtla
					{
						MessageLog::Instance().AddMessage(MessageLog::ecRedundantNearLoopArithmetic, it - instructions.begin()); // specjalnie bez  + 1
					}
				}
			}

		}
		return false;
	}

	//Funkcja testuje operacje arytmetyczne, czy nie sa nadmiarowe
	bool CodeAnalyser::TestArithmetics(CodeTapeIterator& it, const RepairFn2i2& repairCB)
	{
		CodeTapeIterator m, n;
		int sum, ops;

		if (IsArithmeticInstruction(*it))
		{
			ignore_arithmetic_test = true;
			n = std::find_if_not(it, instructions.end(), IsArithmeticInstruction);
			//mamy ci¹g + - 
			//instrukcji musi byc wiêcej niz jedna i wynik ma byc osi¹gniêty najmniejsza liczba instrukcji, czyli bez np suma = 2 dla ++ [ops=2] a nie +-+-++ [ops=5]

			ops = std::count_if(it, n, IsArithmeticInstruction);
			sum = Evaluate(it, n);

			if ((n - it) > 1 && abs(sum) != ops)
			{
				MessageLog::Instance().AddMessage(MessageLog::ecRedundantArithmetic, it - instructions.begin() + 1);
				if (repairCB)
				{
					repairCB(it, n, sum, ops);

				}
			}
		}

		return false;
	}

	//Funkcja testuje operacje ruchu, czy nie sa nadmiarowe
	bool CodeAnalyser::TestRedundantMoves(CodeTapeIterator& it, const RepairFn2i2& repairCB)
	{
		CodeTapeIterator n;
		int sum, ops;

		if (IsMoveInstruction(*it))
		{
			ignore_moves_test = true;
			n = std::find_if_not(it, instructions.end(), IsMoveInstruction);
			//mamy ci¹g < > 
			//instrukcji musi byc wiêcej niz jedna i wynik ma byc osi¹gniêty najmniejsza liczba instrukcji, czyli bez np suma = 2 dla >> [ops=2] a nie <><>>> [ops=5]

			sum = EvaluateMoves(it, n);
			ops = std::count_if(it, n, IsMoveInstruction);

			if ((n - it) > 1 && abs(sum) != ops)
			{
				MessageLog::Instance().AddMessage(MessageLog::ecRedundantMoves, it - instructions.begin() + 1);
				if (repairCB)
				{
					repairCB(it, n, sum, ops);
				}
			}
		}

		return false;
	}

	//Funkcja testuje czy wystapi³y jakies powtórzenia operatorów, np !
	bool CodeAnalyser::TestForRepetition(CodeTapeIterator& it, const RepairFn2& repairCB)
	{
		CodeTapeIterator n;

		if (it + 1 < instructions.end())
		{
			n = std::find_if_not(it + 1, instructions.end(), [&it](const bt_instruction& o) { return o.operation == it->operation; });
			if (n != instructions.end()) //jest jakies powtórzenie
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

	//Funkcja liczy sume wynikaj¹ca z operacji + i - w pewnym ci¹gu instrukcji od start do poprzedzaj¹cego end
	//Nie sa wa¿ne inne operacje 
	int CodeAnalyser::Evaluate(const CodeTapeIterator& begin, const CodeTapeIterator& end) const
	{
		int sum = 0;
		for (CodeTapeIterator it = begin; it < end; ++it)
		{
			switch (it->operation)
			{
			case bt_operation::btoIncrement: ++sum; break;
			case bt_operation::btoDecrement: --sum; break;
			}
		}

		return sum;
	}

	//Funkcja liczy operacje < i >, cel taki jak w Calcule
	//Nie sa wa¿ne inne operacje 
	int CodeAnalyser::EvaluateMoves(const CodeTapeIterator& begin, const CodeTapeIterator& end) const
	{
		int sum = 0;
		for (CodeTapeIterator it = begin; it < end; ++it)
		{
			switch (it->operation)
			{
			case bt_operation::btoMoveRight: ++sum; break;
			case bt_operation::btoMoveLeft:  --sum; break;
			}
		}

		return sum;
	}

	//Funkcja liczy sposób zachowania siê pêtli
	//Return 0 - niepoliczalne, -1: pêtla d¹¿y do zera, 1: pêtla d¹¿y do nieskoñczonoœci
	short CodeAnalyser::GetLoopLimes(const CodeTapeIterator& op) const
	{
		CodeTapeIterator a, n, m;
		std::list<short> limesii; //wyniki podpêtli w kolejnoœci (to wa¿ne jaka kolejnoœæ)
		int s = 0;

		a = instructions.begin() + op->jump;
		m = op + 1;

		if (m == a)
			return 0; //nieskoñczona pêtla

		n = std::find_if_not(m, a, [](const bt_instruction& o) { return IsArithmeticInstruction(o) ||
			IsChangingInstruction(o) == false ||
			o.operation == bt_operation::btoBeginLoop ||
			o.operation == bt_operation::btoEndLoop; });

		if (n == a) //s¹ tylko + - i pêtle i inne niewazne operacje
		{
			while (true)
			{
				n = std::find_if(m, a, [](const bt_instruction& o) { return o.operation == bt_operation::btoBeginLoop ||
					o.operation == bt_operation::btoEndLoop; }); //szukamy pierwszej instrukcji pêtli

				if (n != m) //sa jakies operacje -> [+-+-[
					limesii.push_back(Evaluate(m, a) < 0 ? -1 : 1); //pêtla z przewaga minusów da tutaj -1, inaczej 1, zero liczy sie jak do nieksonczonoœci
				else   //nie ma ¿adnej operacji -> [[
					limesii.push_back(-1); //liczy sie jako normalna zdrowa operacje d¹¿¹ca do zera

				if (n != a) //oho mamy pêtle w œrodku jak¹œ, dawaj jeszcze jej limes
				{
					limesii.push_back(GetLoopLimes(n));
					m = instructions.begin() + n->jump + 1;
				}
				else break; //nie ma pêtli wewnêtrznej
			}
			//mamy tutaj ci¹g 1 i -1 , cos spróbujemy policzyæ

			for (std::list<short>::iterator it = limesii.begin(); it != limesii.end(); ++it)
			{
				if (*it == 0) //którykolwiek element niepoliczalny wyklucza wszystko
					return 0;
			}

			return limesii.back() < 0 ? -1 : 1; //gdy ostatnie d¹¿y w górê do nieskonczonoœci, na pewno siê coœ zjebie 
		}

		return 0; // nie da siê policzyæ limes
	}


	//Funkcja zwraca true, je¿eli ¿¹dana instrukcja znajduje siê w œrodku funkcji
	//Liczymy poprostu nawiasy, inaczej nie bêdziemy pewni
	bool CodeAnalyser::IsWithinFunction(const CodeTapeIterator& op) const
	{
		int opening_bracket_cnt, closing_bracket_cnt;

		opening_bracket_cnt = std::count_if(instructions.begin(), op,
			[](const bt_instruction& op) { return op.operation == bt_operation::btoBeginFunction; });

		closing_bracket_cnt = std::count_if(instructions.begin(), op,
			[](const bt_instruction& op) { return op.operation == bt_operation::btoEndFunction; });

		return opening_bracket_cnt > closing_bracket_cnt;
	}

	//Funkcje modyfikuj¹ linkowania 
	void CodeAnalyser::RelinkCommands(const CodeTapeIterator& start, short n)
	{
		RelinkCommands(start, instructions.end(), n);
	}

	void CodeAnalyser::RelinkCommands(const CodeTapeIterator& start, const CodeTapeIterator& end, short n)
	{
		for (CodeTapeIterator it = start; it < end; ++it)
		{
			if (IsLinkedInstruction(*it) || it->NullJump() == false)
			{
				it->jump -= n;
			}
		}
	}

	//Testuje poprawnoœæ linkujacych komend - ka¿da z nich musi linkowaæ do innej, nie mog¹ byc bez par
	bool CodeAnalyser::TestLinks()
	{
		for (CodeTapeIterator it = instructions.begin(); it < instructions.end(); ++it)
		{
			if (IsLinkedInstruction(*it) && IsLinkedInstruction(instructions.at(it->jump)) == false)
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
	short CodeAnalyser::GetLoopLimes(const std::vector<bt_instruction>::iterator &op) const
	{
		std::vector<bt_instruction>::iterator a, n, m;
		std::list<short> limesii; //wyniki podpêtli w kolejnoœci (to wa¿ne jaka kolejnoœæ)
		int s = 0;

		a = instructions.begin() + op->jump;
		m = op + 1;

		if(m == a)
			return 0; //nieskoñczona pêtla

		n = std::find_if_not(m, a, [&op](const bt_instruction &o){ return IsArithmeticInstruction(o) ||
																						 o.operation == btoBeginLoop ||
																						 o.operation == btoEndLoop; } );

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
					m = instructions.begin() + n->jump + 1;
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
}