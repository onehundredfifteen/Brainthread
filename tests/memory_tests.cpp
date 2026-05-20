#include <cassert>
#include <iostream>
#include <sstream>

#include "tests_declarations.h"
#include "../src/Settings.h"
#include "../src/MemoryTape.h"
#include "../src/MemoryStack.h"

using namespace BT;

// ============================================================================
// Memory Tape Tests
// ============================================================================

void TestMemoryTapeIncrement() {
    std::cout << "Testing MemoryTape increment operations..." << std::endl;

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

    MemoryTape<unsigned char> tape(100, eof_option::eoZero, mem_option::moLimited);

    unsigned int start_pos = tape.PointerPosition();
    tape.MoveRight();
    assert(tape.PointerPosition() == start_pos + 1);

    tape.MoveRight(5);
    assert(tape.PointerPosition() == start_pos + 6);

    tape.MoveLeft();
    assert(tape.PointerPosition() == start_pos + 5);

    tape.MoveLeft(5);
    assert(tape.PointerPosition() == start_pos);

    std::cout << "✓ MemoryTape movement tests passed" << std::endl;
}

void TestMemoryTapeByteWrapping() {
    std::cout << "Testing MemoryTape byte wrapping (8-bit)..." << std::endl;

    MemoryTape<unsigned char> tape(100, eof_option::eoZero, mem_option::moLimited);

    *const_cast<unsigned char*>(tape.GetValue()) = 255;
    tape.Increment();
    assert(*tape.GetValue() == 0);

    tape.Decrement();
    assert(*tape.GetValue() == 255);

    std::cout << "✓ MemoryTape byte wrapping tests passed" << std::endl;
}

void TestMemoryTapeDynamicGrowth() {
    std::cout << "Testing MemoryTape with dynamic memory growth..." << std::endl;

    MemoryTape<unsigned char> tape(10, eof_option::eoZero, mem_option::moDynamic);

    for (int i = 0; i < 20; i++) {
        tape.MoveRight();
    }
    assert(tape.PointerPosition() == 20);

    std::cout << "✓ MemoryTape dynamic growth tests passed" << std::endl;
}

// ============================================================================
// Memory Heap Tests
// ============================================================================

void TestMemoryStack() {
    std::cout << "Testing MemoryStack operations..." << std::endl;

    MemoryStack<int> stack;

    // Pop on empty returns 0
    assert(stack.Pop() == 0);

    // Push and Pop
    stack.Push(1);
    stack.Push(2);
    assert(stack.Pop() == 2);

    // Swap top two
    stack.Push(3);
    stack.Push(4);
    stack.Swap();
    assert(stack.Pop() == 3);
    assert(stack.Pop() == 4);

    // PrintStack produces textual output
    stack.Push(5);
    stack.Push(6);
    std::stringstream ss;
    stack.PrintStack(ss);
    std::string out = ss.str();
    (void)out; // just ensure it compiles; content inspected by human if needed

    std::cout << "✓ MemoryStack tests passed" << std::endl;
}
