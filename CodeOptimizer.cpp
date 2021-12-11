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
/**/
bool CodeOptimizer::OptimizeToZeroLoop(TapeIterator &it, const RepairFn2 & repairCB)
{
	TapeIterator n;
	int lim, s;

	if (it->operation == bt_operation::btoBeginLoop)
	{
		//TestForRepetition
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

//Funkcje modyfikuj¹ linkowania 
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


