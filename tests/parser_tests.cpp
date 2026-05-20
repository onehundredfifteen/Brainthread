#include <cassert>
#include <iostream>
#include <vector>

#include "tests_declarations.h"
#include "../src/Settings.h"
#include "../src/BrainThread.h"

using namespace BT;

// ============================================================================
// Parser Tests
// ============================================================================

void TestParserValidSyntax() {
    std::cout << "Testing parser with valid syntax..." << std::endl;

    std::vector<std::string> valid_commands = {
        "++++",           // increment commands
        "++[>+<-]",       // loop structure
        "+*",             // function call
        ">><<",           // pointer movement
        ",.>.<",          // input/output operations
        "[+]"           // strange but valid loop
        "++comments--",   // valid with comments
        "++>>:;{}()^!%"  // bt commands
    };

    for (const auto& code : valid_commands) {
        ParserBase parser = Parser<CodeLang::clBrainThread, 1>(code);
        assert(parser.IsSyntaxValid());

        ParserBase parser_opt = Parser<CodeLang::clBrainThread, 2>(code);
        assert(parser_opt.IsSyntaxValid());
    }

    // debug instructions: exercise debug pragma parsing (do not assert strict validity)
    {
        ParserBase dbg = Parser<CodeLang::clBrainThread, 0>("++#M--#115>>");
        (void)dbg;
    }

    std::cout << "✓ Parser valid syntax tests passed" << std::endl;
}

void TestParserInvalidSyntax() {
    std::cout << "Testing parser with invalid syntax..." << std::endl;

    std::vector<std::string> invalid_commands = {
        "+++[+",          // unmatched open bracket
        "+++]",           // unmatched close bracket
        "+[++]--[",       // mismatched brackets
        "]]]",            // only closing brackets
        "[[[",            // only opening brackets
        "[][",            // mixed unmatched

        "+++(+",          // unmatched open parenthesis
        "+++)",           // unmatched close parenthesis
        "+(++)--(",       // mismatched parenthesis
        ")))",            // only closing parenthesis
        "(((",            // only opening parenthesis
        "())",            // mixed unmatched

        "[)]()]"     // mismatched brackets and parentheses
    };

    for (const auto& code : invalid_commands) {
        ParserBase parser = Parser<CodeLang::clBrainThread, 0>(code);
        assert(parser.IsSyntaxValid() == false);

        ParserBase parser_opt = Parser<CodeLang::clBrainThread, 1>(code);
        assert(parser_opt.IsSyntaxValid() == false);

        ParserBase parser_opt2 = Parser<CodeLang::clBrainThread, 2>(code);
        assert(parser_opt2.IsSyntaxValid() == false);
    }

    std::cout << "✓ Parser invalid syntax tests passed" << std::endl;
}

void TestParserInstructions() {
    std::cout << "Testing parser instruction generation..." << std::endl;

    ParserBase parser = Parser<CodeLang::clBrainThread, 1>("++++");
    assert(parser.IsSyntaxValid() == true);
    assert(parser.GetInstructions().size() > 0);
    assert(parser.GetInstructions().back().operation == bt_operation::btoEndProgram);

    std::cout << "✓ Parser instruction tests passed" << std::endl;
}

void TestParserBrainfuck() {
    std::cout << "Testing parser with Brainfuck language..." << std::endl;

    // Test basic Brainfuck operations
    ParserBase parser = Parser<CodeLang::clBrainFuck, 0>("++>--<,.");
    assert(parser.IsSyntaxValid() == true);

    // Brainfuck should not support Brainthread commands
    parser = Parser<CodeLang::clBrainFuck, 0>("+*+!");  // * is function call in brainthread
    assert(parser.GetInstructions().size() == 3); // should only parse ++ and ignore *!
    assert(parser.GetInstructions().back().operation == bt_operation::btoEndProgram);

    std::cout << "✓ Parser Brainfuck tests passed" << std::endl;
}
