#include <cassert>
#include <iostream>

#include "tests_declarations.h"
#include "../src/Settings.h"
#include "../src/BrainThread.h"

using namespace BT;

void TestInterpreterBasicExecution() {
    std::cout << "Testing basic interpreter execution..." << std::endl;

    Settings settings;
    ParserBase parser = Parser<CodeLang::clBrainThread, 0>("++++");

    auto interpreter = ProduceInterpreter(settings);
    interpreter->Run(parser.GetInstructions());

    std::cout << "✓ Basic interpreter smoke tests passed" << std::endl;
}

void TestInterpreterExecution() {
    std::cout << "Testing basic interpreter execution..." << std::endl;

    Settings settings;
    ParserBase parser = Parser<CodeLang::clBrainThread, 0>("++!>^");

    auto interpreter = ProduceInterpreter(settings);
    interpreter->Run(parser.GetInstructions());

    std::cout << "✓ Basic interpreter smoke tests passed" << std::endl;
}