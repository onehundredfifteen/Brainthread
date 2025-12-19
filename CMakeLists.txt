cmake_minimum_required(VERSION 3.10.0)
project(Brainthread VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Brainthread src/BrainThread.cpp src/BrainHelp.cpp src/BrainThreadExceptions.cpp src/BrainThreadProcess.cpp src/BrainThreadRuntimeException.cpp src/CodeAnalyser.cpp src/CodeTape.cpp src/DebugLogStream.cpp src/FunctionHeap.cpp src/Interpreter.cpp src/MemoryHeap.cpp src/MemoryTape.cpp src/MessageLog.cpp src/Parser.cpp src/Settings.cpp src/main.cpp)

include(CTest)
enable_testing()

add_executable(bttest tests/basic_tests.cpp src/BrainThread.cpp src/BrainHelp.cpp src/BrainThreadExceptions.cpp src/BrainThreadProcess.cpp src/BrainThreadRuntimeException.cpp src/CodeAnalyser.cpp src/CodeTape.cpp src/DebugLogStream.cpp src/FunctionHeap.cpp src/Interpreter.cpp src/MemoryHeap.cpp src/MemoryTape.cpp src/MessageLog.cpp src/Parser.cpp src/Settings.cpp)
add_test(NAME basics COMMAND bttest)