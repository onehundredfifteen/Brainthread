cmake_minimum_required(VERSION 3.10.0)
project(Brainthread VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Brainthread src/BrainThread.cpp src/BrainThreadExceptions.cpp src/BrainThreadProcess.cpp src/BrainThreadRuntimeException.cpp src/CodeAnalyser.cpp src/DebugLogStream.cpp src/FunctionStack.cpp src/Interpreter.cpp src/MemoryStack.cpp src/MemoryTape.cpp src/MessageLog.cpp src/Parser.cpp src/Settings.cpp infoAndHelp.cpp main.cpp)

include(CTest)
enable_testing()

add_executable(bttest
	tests/basic_tests.cpp
	tests/parser_tests.cpp
	tests/memory_tests.cpp
	tests/settings_tests.cpp
	tests/interpreter_tests.cpp
	src/BrainThread.cpp
	src/BrainThreadExceptions.cpp
	src/BrainThreadProcess.cpp
	src/BrainThreadRuntimeException.cpp
	src/CodeAnalyser.cpp
	src/DebugLogStream.cpp
	src/FunctionStack.cpp
	src/Interpreter.cpp
	src/MemoryStack.cpp
	src/MemoryTape.cpp
	src/MessageLog.cpp
	src/Parser.cpp
	src/Settings.cpp
)
add_test(NAME basics COMMAND bttest)