#include "CodeOptimizer.h"

CodeOptimizer::CodeOptimizer(std::vector<CodeTape::bt_instruction> *instructions, coLevel _level)
	: CodeAnalyser(instructions) , level(_level)
{
	
}

CodeOptimizer::~CodeOptimizer(void)
{
}

void CodeOptimizer::Optimize(std::queue<unsigned int> &optimizer_entrypoints)
{
	CodeIterator it = instructions->begin();


	while (optimizer_entrypoints.empty() == false)
	{
		it = instructions->begin() + optimizer_entrypoints.front();
		optimizer_entrypoints.pop();

		if (it->operation == CodeTape::btoBeginLoop) {
			if (it + 2 < instructions->end() && 
				(it + 1)->operation == CodeTape::btoDecrement &&
				(it + 2)->operation == CodeTape::btoEndLoop)
			{
				it->operation = CodeTape::btoOPT_NoOperation;
				(it + 1)->operation = CodeTape::btoOPT_NoOperation;
				(it + 2)->operation = CodeTape::btoOPT_SetCellToZero;
			}
		}
		/*else if (it->operation == CodeTape::btoMoveLeft) {
			if (it + 2 < instructions->end()) {
				TestForRepetition()
			}
		}*/
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
bool CodeOptimizer::OptimizeToZeroLoop(CodeIterator &it, const RepairFn2 & repairCB)
{
	CodeIterator n;
	int lim, s;

	if (it->operation == CodeTape::btoBeginLoop)
	{
		//TestForRepetition
		lim = GetLoopLimes(it);

		if (lim > 0) //wolna p�tla d���ca do niesko�czono�ci
		{
			MessageLog::Instance().AddMessage(MessageLog::ecSlowLoop, it - instructions->begin() + 1);
		}

		//teraz czy przed p�tl� s� sensowne operacje
		if (it > instructions->begin()) //ofc tylko dla dalszych instrukcji
		{
			for (n = it - 1; n > instructions->begin(); --n)
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
					MessageLog::Instance().AddMessage(MessageLog::ecRedundantNearLoopArithmetic, it - instructions->begin()); // specjalnie bez  + 1
				}
			}
		}

	}
	return false;
}


