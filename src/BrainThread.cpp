#include <memory>
#include <chrono>
#include <sstream>

#include "BrainThread.h"
#include "Interpreter.h"
#include "Parser.h"
#include "CodeAnalyser.h"

using namespace BT;

 void RunProgram(const Settings& flags) 
 {
        MessageLog::Instance().SetMessageLevel(flags.OP_message);
        auto start = std::chrono::system_clock::now();

        ParserBase parser = ParseCode(flags.OP_source_code, flags);

        if (flags.OP_analyse || flags.OP_optimize) {
            RunAnalyser(parser, flags);
        }

        auto exec_start = std::chrono::system_clock::now();
        if (parser.IsSyntaxValid() && flags.OP_execute) {
            ProduceInterpreter(flags)->Run(parser.GetInstructions());
        }

        if (flags.OP_message == MessageLog::MessageLevel::mlAll) {
            auto end = std::chrono::system_clock::now();
            auto elapsed = std::chrono::duration_cast <std::chrono::milliseconds> (exec_start - start).count();
            auto exec_elapsed = std::chrono::duration_cast <std::chrono::milliseconds> (end - exec_start).count();

            MessageLog::Instance().AddInfo("Parsing completed in " + std::to_string(elapsed) + " miliseconds");
            MessageLog::Instance().AddInfo("Execution completed in " + std::to_string(exec_elapsed) + " miliseconds");
        }
    } 
namespace BT
{
    ParserBase ParseCode(const std::string& code, const Settings& flags)
    {
        switch (flags.OP_language)
        {
            case CodeLang::clBrainThread:
            {
                if (flags.OP_analyse) return Parser<CodeLang::clBrainThread, 0>(code);
                else if (flags.OP_optimize) return Parser<CodeLang::clBrainThread, 2>(code);
                else return Parser<CodeLang::clBrainThread, 1>(code);
            }
            break;
            case CodeLang::clPBrain:
            {
                if (flags.OP_analyse) return Parser<CodeLang::clPBrain, 0>(code);
                else if (flags.OP_optimize) return Parser<CodeLang::clPBrain, 2>(code);
                else return Parser<CodeLang::clPBrain, 1>(code);
            }
            break;
            case CodeLang::clBrainFork:
            {
                if (flags.OP_analyse) return Parser<CodeLang::clBrainFork, 0>(code);
                else if (flags.OP_optimize) return Parser<CodeLang::clBrainFork, 2>(code);
                else return Parser<CodeLang::clBrainFork, 1>(code);
            }
            case CodeLang::clBrainFuck:
            default:
            {
                if (flags.OP_analyse) return Parser<CodeLang::clBrainFuck, 0>(code);
                else if (flags.OP_optimize) return Parser<CodeLang::clBrainFuck, 2>(code);
                else return Parser<CodeLang::clBrainFuck, 1>(code);
            }
        }
    }

    std::unique_ptr<InterpreterBase> ProduceInterpreter(const Settings& flags)
    {
        switch (flags.OP_cellsize)
        {
            case cellsize_option::cs16: return std::make_unique<Interpreter<short>>(flags.OP_mem_behavior, flags.OP_eof_behavior, flags.OP_mem_size);
            break;
            case cellsize_option::cs32: return std::make_unique <Interpreter<int>>(flags.OP_mem_behavior, flags.OP_eof_behavior, flags.OP_mem_size);
            break;
            case  cellsize_option::csu8: return std::make_unique<Interpreter<unsigned char>>(flags.OP_mem_behavior, flags.OP_eof_behavior, flags.OP_mem_size);
            break;
            case  cellsize_option::csu16: return std::make_unique<Interpreter<unsigned short>>(flags.OP_mem_behavior, flags.OP_eof_behavior, flags.OP_mem_size);
            break;
            case  cellsize_option::csu32: return std::make_unique<Interpreter<unsigned int>>(flags.OP_mem_behavior, flags.OP_eof_behavior, flags.OP_mem_size);
            break;
            case cellsize_option::cs8: 
            default: return std::make_unique<Interpreter<char>>(flags.OP_mem_behavior, flags.OP_eof_behavior, flags.OP_mem_size);
        }
    }

    void RunAnalyser(ParserBase& parser, const Settings& flags)
    {
        try
        {
            if (parser.IsSyntaxValid()) //syntax looks fine
            {
                MessageLog::Instance().AddInfo("Parser: Syntax is valid");
                
                if (flags.OP_analyse)
                {
                    CodeAnalyser analyser(parser);
                    flags.OP_optimize ? analyser.Repair() : analyser.Analyse();

                    if (analyser.isCodeValid())
                    {
                        if (analyser.RepairedSomething() == true)
                            MessageLog::Instance().AddInfo("Some bugs have been successfully fixed");

                        MessageLog::Instance().AddInfo("Code Analyser: Code is sane");
                    }
                    else
                    {
                        MessageLog::Instance().AddInfo("Code Analyser: Code has warnings");
                    }
                }
            }
            else
            {
                MessageLog::Instance().AddMessage("Parser: Invalid syntax");
            }

        }
        catch (...)
        {
            MessageLog::Instance().AddMessage(MessageLog::ErrCode::ecUnknownError, "Internal method RunAnalyser");
        }
    }

}


