#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <time.h>

// Platform detection
#ifdef _WIN32
    #include <windows.h>
    #include <dbghelp.h>
#ifdef _MSC_VER
    #pragma comment(lib, "dbghelp.lib")
#endif

#else
    #include <execinfo.h>
    #include <unistd.h>
#endif

#ifndef LOG_H
#define LOG_H

class Log
{
private:
    static FILE* currentLog;

    static void SEGFAULT_HANDLER(int SIG);
    static std::string getDate();

public:
    static void initLog(std::string logName);
    static void write(std::string logEntry);
    static void closeLog();
};

#endif