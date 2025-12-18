cmake_minimum_required(VERSION 3.10.0)
project(Brainthread VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)


add_executable(Brainthread BrainHelp.cpp BrainThreadExceptions.cpp BrainThreadProcess.cpp BrainThreadRuntimeException.cpp CodeAnalyser.cpp CodeTape.cpp DebugLogStream.cpp FunctionHeap.cpp Interpreter.cpp MemoryHeap.cpp MemoryTape.cpp MessageLog.cpp Parser.cpp Settings.cpp main.cpp)

include(CTest)
enable_testing()

