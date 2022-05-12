// TestExtension.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "TestExtension.h"

extern "C"
{
	__declspec (dllexport) void __stdcall RVExtension(char* output, int outputSize, const char* function);
}

void __stdcall RVExtension(char* output, int outputSize, const char* function)
{
	strncpy_s(output, outputSize, "IT WORKS!", _TRUNCATE);
}
