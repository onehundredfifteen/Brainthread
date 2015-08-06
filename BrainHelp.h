#pragma once

#include <iostream>
#include <string>

#ifndef BT_VERSION
	#define BT_VERSION "0.5a"
#endif

void PrintBrainThreadInfo();
void PrintBrainThreadInfoEx();
void ShowUsage();
void ShowHelp(std::string help_opt);