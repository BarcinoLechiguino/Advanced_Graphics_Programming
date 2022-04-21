#ifdef _WIN32
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

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