#include <Windows.h>

#include "globals.h"

// It logs a string to whichever outputs are configured in the platform layer. 
// By default, the string is printed in the output console of VisualStudio.
void LogString(const char* str)
{   
    #ifdef _WIN32
    OutputDebugStringA(str);
    OutputDebugStringA("\n");
    #else
    fprintf(stderr, "%s\n", str);
    #endif 
}