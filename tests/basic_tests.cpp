#include <cassert>
#include <sstream>
#include <iostream>

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
        "[+]",            // strange but valid loop
        "++comments--",   // valid with comments
        "++>>:;{}()^%&~"  //bt commands
    };
    
    for (const auto& code : valid_commands) {
        ParserBase parser = Parser<CodeLang::clBrainThread, 1>(code);
        assert(parser.IsSyntaxValid() == true);
        
        ParserBase parser_opt = Parser<CodeLang::clBrainThread, 2>(code);
        assert(parser_opt.IsSyntaxValid() == true);
    }

        //debug instructions
    ParserBase parser = Parser<CodeLang::clBrainThread, 0>("++#M--#115>>");
        assert(parser.IsSyntaxValid() == true);
    
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
    parser = Parser<CodeLang::clBrainFuck, 0>("+*");  // * is function call in brainthread
    // This should still parse (unknown chars are ignored in some implementations)
    
    std::cout << "✓ Parser Brainfuck tests passed" << std::endl;
}

// ============================================================================
// Memory Tape Tests
// ============================================================================

void TestMemoryTapeIncrement() {
    std::cout << "Testing MemoryTape increment operations..." << std::endl;
    
    Settings settings;
    MemoryTape<unsigned char> tape(100, eof_option::eoZero, mem_option::moLimited);
    
    tape.Increment();
    assert(*tape.GetValue() == 1);
    
    tape.Increment(5);
    assert(*tape.GetValue() == 6);
    
    tape.Decrement();
    assert(*tape.GetValue() == 5);
    
    tape.Decrement(3);
    assert(*tape.GetValue() == 2);
    
    std::cout << "✓ MemoryTape increment tests passed" << std::endl;
}

void TestMemoryTapeMovement() {
    std::cout << "Testing MemoryTape movement operations..." << std::endl;
    
    Settings settings;
    MemoryTape<unsigned char> tape(100, eof_option::eoZero, mem_option::moLimited);
    
    // Move right and verify position
    unsigned int start_pos = tape.PointerPosition();
    tape.MoveRight();
    assert(tape.PointerPosition() == start_pos + 1);
    
    tape.MoveRight(5);
    assert(tape.PointerPosition() == start_pos + 6);
    
    // Move left and verify position
    tape.MoveLeft();
    assert(tape.PointerPosition() == start_pos + 5);
    
    tape.MoveLeft(5);
    assert(tape.PointerPosition() == start_pos);
    
    std::cout << "✓ MemoryTape movement tests passed" << std::endl;
}

void TestMemoryTapeByteWrapping() {
    std::cout << "Testing MemoryTape byte wrapping (8-bit)..." << std::endl;
    
    Settings settings;
    MemoryTape<unsigned char> tape(100, eof_option::eoZero, mem_option::moLimited);
    
    // Test overflow
    *const_cast<unsigned char*>(tape.GetValue()) = 255;
    tape.Increment();
    assert(*tape.GetValue() == 0);  // Should wrap around
    
    // Test underflow
    tape.Decrement();
    assert(*tape.GetValue() == 255);  // Should wrap around
    
    std::cout << "✓ MemoryTape byte wrapping tests passed" << std::endl;
}

void TestMemoryTapeDynamicGrowth() {
    std::cout << "Testing MemoryTape with dynamic memory growth..." << std::endl;
    
    Settings settings;
    MemoryTape<unsigned char> tape(10, eof_option::eoZero, mem_option::moDynamic);
    
    // Try to move beyond initial size (should grow)
    for (int i = 0; i < 20; i++) {
        tape.MoveRight();
    }
    // If we get here without crashing, dynamic growth works
    assert(tape.PointerPosition() == 20);
    
    std::cout << "✓ MemoryTape dynamic growth tests passed" << std::endl;
}

// ============================================================================
// Memory Heap Tests
// ============================================================================

void TestMemoryHeap() {
    std::cout << "Testing MemoryHeap operations..." << std::endl;

    MemoryHeap<int> heap;

    // Pop on empty returns 0
    assert(heap.Pop() == 0);

    // Push and Pop
    heap.Push(1);
    heap.Push(2);
    assert(heap.Pop() == 2);

    // Swap top two
    heap.Push(3);
    heap.Push(4);
    heap.Swap();
    // After swap, top should be the previous second element (3)
    assert(heap.Pop() == 3);
    assert(heap.Pop() == 4);

    // PrintStack produces textual output
    heap.Push(5);
    heap.Push(6);
    std::stringstream ss;
    heap.PrintStack(ss);
    std::string out = ss.str();
    assert(out.find("Memory stack") != std::string::npos || out.find("Memory stack") != std::string::npos);

    std::cout << "✓ MemoryHeap tests passed" << std::endl;
}

// ============================================================================
// Basic Interpreter Tests
// ============================================================================

void TestInterpreterBasicExecution() {
    std::cout << "Testing basic interpreter execution..." << std::endl;
    
    Settings settings;
    ParserBase parser = Parser<CodeLang::clBrainThread, 0>("++++");
    
    // Just verify it runs without throwing
    auto interpreter = ProduceInterpreter(settings);
    interpreter->Run(parser.GetInstructions());
    
    std::cout << "✓ Basic interpreter smoke tests passed" << std::endl;
}

// ============================================================================
// Settings Tests
// ============================================================================

void TestSettingsDefaults() {
    std::cout << "Testing Settings default values..." << std::endl;
    
    Settings settings;
    assert(settings.OP_cellsize == cellsize_option::cs8);
    assert(settings.OP_eof_behavior == eof_option::eoZero);
    assert(settings.OP_mem_behavior == mem_option::moLimited);
    
    std::cout << "✓ Settings default tests passed" << std::endl;
}

void TestSettingsCellsize() {
    std::cout << "Testing Settings cellsize options..." << std::endl;
    
    // Test cellsize parsing
    Settings settings;
    const char* argv[] = {"brainthread", "-c", "16"};
    GetOpt::GetOpt_pp ops(3, const_cast<char**>(argv));
    settings.InitFromArguments(ops);
    assert(settings.OP_cellsize == cellsize_option::cs16);
    
    Settings settings2;
    const char* argv2[] = {"brainthread", "-c", "32"};
    GetOpt::GetOpt_pp ops2(3, const_cast<char**>(argv2));
    settings2.InitFromArguments(ops2);
    assert(settings2.OP_cellsize == cellsize_option::cs32);
    
    std::cout << "✓ Settings cellsize tests passed" << std::endl;
}

void TestSettingsMemoryBehavior() {
    std::cout << "Testing Settings memory behavior options..." << std::endl;
    
    Settings settings;
    const char* argv[] = {"brainthread", "-b", "dynamic"};
    GetOpt::GetOpt_pp ops(3, const_cast<char**>(argv));
    settings.InitFromArguments(ops);
    assert(settings.OP_mem_behavior == mem_option::moDynamic);
    
    Settings settings2;
    const char* argv2[] = {"brainthread", "-b", "tapeloop"};
    GetOpt::GetOpt_pp ops2(3, const_cast<char**>(argv2));
    settings2.InitFromArguments(ops2);
    assert(settings2.OP_mem_behavior == mem_option::moContinuousTape);
    
    std::cout << "✓ Settings memory behavior tests passed" << std::endl;
}

void TestSettingsLanguage() {
    std::cout << "Testing Settings language options..." << std::endl;
    
    Settings settings;
    const char* argv[] = {"brainthread", "-l", "brainfuck"};
    GetOpt::GetOpt_pp ops(3, const_cast<char**>(argv));
    settings.InitFromArguments(ops);
    assert(settings.OP_language == CodeLang::clBrainFuck);
    
    Settings settings2;
    const char* argv2[] = {"brainthread", "-l", "brainthread"};
    GetOpt::GetOpt_pp ops2(3, const_cast<char**>(argv2));
    settings2.InitFromArguments(ops2);
    assert(settings2.OP_language == CodeLang::clBrainThread);
    
    std::cout << "✓ Settings language tests passed" << std::endl;
}

// ============================================================================
// Test Runner
// ============================================================================

int main()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "Running Brainthread Test Suite" << std::endl;
    std::cout << "========================================\n" << std::endl;

    try {
        // Parser tests
        TestParserValidSyntax();
        TestParserInvalidSyntax();
        TestParserInstructions();
        TestParserBrainfuck();

        // Memory tests
        TestMemoryTapeIncrement();
        TestMemoryTapeMovement();
        TestMemoryTapeByteWrapping();
        TestMemoryTapeDynamicGrowth();
        TestMemoryHeap();

        // Interpreter tests
        TestInterpreterBasicExecution();

        // Settings tests
        TestSettingsDefaults();
        TestSettingsCellsize();
        TestSettingsMemoryBehavior();
        TestSettingsLanguage();

        std::cout << "\n========================================" << std::endl;
        std::cout << "All tests passed! ✓" << std::endl;
        std::cout << "========================================\n" << std::endl;

        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << "\n❌ Test failed with exception: " << ex.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "\n❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
}