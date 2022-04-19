#ifndef __GLOBALS_H__
#define __GLOBALS_H__

void LogString(const char* str);

#define ILOG(...)                     \
{                                     \
    char logBuffer[1024] = {};        \
    sprintf(logBuffer, __VA_ARGS__);  \
    LogString(logBuffer);             \
}

#define ELOG(...) ILOG(__VA_ARGS__)

#define ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))

#define ASSERT(condition, message) assert((condition) && message)

#define KB(count) (1024*(count))
#define MB(count) (1024*KB(count))
#define GB(count) (1024*MB(count))

#define PI  3.14159265359f
#define TAU 6.28318530718f

#endif // !__GLOBALS_H__