#include <cassert>

#include "../src/Settings.h"
#include "../src/BrainThread.h"

using namespace BT;

ParserBase GetParser(std::string code){
    return Parser<CodeLang::clBrainThread, 1>(code);
}

int main()
{
    Settings settings;
    ParserBase parser = GetParser("++++");

    assert(parser.IsSyntaxValid() == true);

    ParserBase parser2 = GetParser("+++[+");

    assert(parser2.IsSyntaxValid() == false);

	ProduceInterpreter(settings)->Run(parser.GetInstructions());

    return 0;
}