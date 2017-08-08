#pragma once

#include <iostream>
#include <string>

#ifndef BT_VERSION
	#define BT_VERSION "1.2"
#endif

void PrintBrainThreadInfo(void);
void PrintBrainThreadInfoEx(void);
void ShowUsage(void);
void ShowHelp(std::string help_opt);
void ShowInfo(void);