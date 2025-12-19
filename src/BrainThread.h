#pragma once

#include "Settings.h"
#include "Interpreter.h"
#include "Parser.h"

//Main method
void RunProgram(const BT::Settings& flags);

namespace BT {

    ParserBase ParseCode(const std::string& code, const Settings& flags);

    void RunAnalyser(ParserBase& parser, const Settings& flags);

    std::unique_ptr<InterpreterBase> ProduceInterpreter(const Settings& flags);
}



