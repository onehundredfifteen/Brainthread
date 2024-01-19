#pragma once

#include <string>

#ifndef BT_VERSION
	#define BT_VERSION "2.0"
#endif
#ifndef BT_DATE
	#define BT_DATE "2024"
#endif

void PrintBrainThreadInfo(void);
void PrintBrainThreadInfoEx(void);
void ShowUsage(const std::string& exe_name);
void ShowInfo(void);