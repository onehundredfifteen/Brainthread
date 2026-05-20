#pragma once

// Declarations for test functions implemented across multiple test files
void TestParserValidSyntax();
void TestParserInvalidSyntax();
void TestParserInstructions();
void TestParserBrainfuck();

void TestMemoryTapeIncrement();
void TestMemoryTapeMovement();
void TestMemoryTapeByteWrapping();
void TestMemoryTapeDynamicGrowth();
void TestMemoryStack();

void TestInterpreterBasicExecution();

void TestSettingsDefaults();
void TestSettingsCellsize();
void TestSettingsMemoryBehavior();
void TestSettingsLanguage();
