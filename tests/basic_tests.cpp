#include <iostream>

#include "tests_declarations.h"

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
        TestMemoryStack();

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
