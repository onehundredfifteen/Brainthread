#include <cassert>
#include <iostream>

#include "tests_declarations.h"
#include "../src/Settings.h"

using namespace BT;

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
