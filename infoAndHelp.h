#pragma once

#include <string>
#include <ostream>

#ifndef BT_VERSION
	#define BT_VERSION "2.1.115"
#endif
#ifndef BT_DATE
	#define BT_DATE "2026"
#endif

void PrintBrainThreadInfo(std::ostream& out);
void PrintBrainThreadInfoEx(std::ostream& out);
void ShowUsage(std::ostream& out, const std::string& exe_name);
void ShowInfo(std::ostream& out);