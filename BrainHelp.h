#pragma once

#include <iostream>
#include <string>

#ifndef BT_VERSION
	#define BT_VERSION "2.0"
#endif
#ifndef BT_DATE
	#define BT_DATE "2024"
#endif

void PrintBrainThreadInfo(void);
void PrintBrainThreadInfoEx(void);
void ShowUsage(void);
void ShowHelp(std::string help_opt);
void ShowInfo(void);