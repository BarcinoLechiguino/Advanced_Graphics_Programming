#ifndef __WINDOWS_INCLUDES_H__
#define __WINDOWS_INCLUDES_H__

#ifdef _WIN32
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#endif // !__WINDOWS_INCLUDES_H__
