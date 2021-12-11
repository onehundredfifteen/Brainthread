#include "CodeOptimizer.h"

CodeOptimizer::CodeOptimizer(std::list<unsigned int>& _entrypoints, std::vector<CodeTape::bt_instruction> &instructions, coLevel _level)
	: optimizer_entrypoints(_entrypoints), CodeAnalyser(instructions) , level(_level)
{
	
}

CodeOptimizer::~CodeOptimizer(void)
{
}

void CodeOptimizer::Optimize()
{
	TapeIterator it = instructions.begin();

	//funkcje naprawcze
	auto repairRepetition = [this](TapeIterator& _it, TapeIterator& n) {
		this->RelinkCommands(_it, (n - _it - 1));
		_it->repetitions = (n - _it);
		_it = instructions.erase(_it + 1, n);
		//++repaired_issues;
	};

	for(unsigned int n : optimizer_entrypoints)
	{
		it = instructions.begin() + n;

		if (it->operation == bt_operation::btoBeginLoop) {
			if (it + 2 < instructions.end() && 
				(it + 1)->operation == bt_operation::btoDecrement &&
				(it + 2)->operation == bt_operation::btoEndLoop)
			{
				it->operation = bt_operation::btoOPT_NoOperation;
				(it + 1)->operation = bt_operation::btoOPT_NoOperation;
				(it + 2)->operation = bt_operation::btoOPT_SetCellToZero;
			}
		}
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
/**/
bool CodeOptimizer::OptimizeToZeroLoop(TapeIterator &it, const RepairFn2 & repairCB)
{
	TapeIterator n;
	int lim, s;

	if (it->operation == bt_operation::btoBeginLoop)
	{
		//TestForRepetition
		lim = GetLoopLimes(it);

		if (lim > 0) //wolna p�tla d���ca do niesko�czono�ci
		{
			MessageLog::Instance().AddMessage(MessageLog::ecSlowLoop, it - instructions.begin() + 1);
		}

		//teraz czy przed p�tl� s� sensowne operacje
		if (it > instructions.begin()) //ofc tylko dla dalszych instrukcji
		{
			for (n = it - 1; n > instructions.begin(); --n)
			{
				if (IsArithmeticInstruction(*n) == false)
					break; //kr�ci si� w ty�, a� znajdzie inna instrukcje + - albo pocz�tek ci�gu
			}

			if (it - 1 != n) //jest znaleziona przynajmniej jedna instrukcja + -
			{
				//tutaj n oznacza pierwsza nie + - instrukcje przed p�tl�
				s = Evaluate(n + 1, it);

				if (lim != 0 && (lim > 0 || (lim < 0 && s < 0))) //je�eli p�tla jest policzalna, a sekwencja przed ni� da�y inaczej ni� p�tla
				{
					MessageLog::Instance().AddMessage(MessageLog::ecRedundantNearLoopArithmetic, it - instructions.begin()); // specjalnie bez  + 1
				}
			}
		}

	}
	return false;
}

//Funkcje modyfikuj� linkowania 
void CodeOptimizer::RelinkCommands(const TapeIterator& start, short n)
{
	RelinkCommands(start, instructions.end(), n);
}

void CodeOptimizer::RelinkCommands(const TapeIterator& start, const TapeIterator& end, short n)
{
	for (TapeIterator it = start; it < end; ++it)
	{
		if (IsLinkedInstruction(*it) || it->NullJump() == false)
		{
			it->jump -= n;
		}
	}
}


