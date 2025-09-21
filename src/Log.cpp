#include "../headers/Log.h"

FILE *Log::currentLog = nullptr;



void Log::SEGFAULT_HANDLER(int SIG)
{
    printf("A FATAL EXCEPTION HAS OCCURRED\n");


    if (currentLog != nullptr)
    {
        DWORD64 moduleBase = (DWORD64)GetModuleHandle(NULL);
        
        fprintf(currentLog, "[%s] - A SEGMENTATION FAULT HAS BEEN TRIGGERED - ERROR CODE %d.\n", Log::getDate().c_str(), SIG);
        fprintf(currentLog, "[%s] - MODULE BASE - (0x%llx) %d. - Good luck!\n", Log::getDate().c_str(),moduleBase );
        fflush(currentLog);

#ifdef _WIN32
        // Windows implementation using DbgHelp
        const int max_frames = 16;
        void* stack[max_frames];
        
        HANDLE process = GetCurrentProcess();
        SymInitialize(process, NULL, TRUE);
        
        WORD frames = CaptureStackBackTrace(0, max_frames, stack, NULL);
        
        for (int i = 0; i < frames; i++) {
            DWORD64 address = (DWORD64)(stack[i]);
            
            char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
            PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = MAX_SYM_NAME;
            
            if (SymFromAddr(process, address, NULL, symbol)) {
                fprintf(currentLog, "[%d] %s (0x%llx)\n", i, symbol->Name, address);
            } else {
                fprintf(currentLog, "[%d] <unknown> (0x%llx)\n", i, address);
            }
        }
        
        SymCleanup(process);
#else
        //posix
        void *stackTrace[16];
        size_t size = backtrace(stackTrace, 16);
        backtrace_symbols_fd(stackTrace, size, fileno(currentLog));
#endif

        fflush(currentLog);
        fclose(currentLog);
    }
    exit(-1);
}

void Log::write(std::string logEntry)
{
    fprintf(currentLog, "[%s] - %s\n", Log::getDate().c_str(), logEntry.c_str());
    fflush(currentLog);
}

std::string Log::getDate()
{
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    char buffer[80];
    strftime(buffer, 80, "%I:%M:%S %p", timeinfo);
    return std::string(buffer);
}

void Log::initLog(std::string name)
{
    std::string fName = "./Logs/";
    fName.append(name.c_str());
    currentLog = fopen(fName.c_str(), "w+");

    if (currentLog != NULL)
    {
        signal(SIGSEGV, SEGFAULT_HANDLER);
        printf("Debug log started at %s.\n", fName.c_str());

        fprintf(currentLog, "Log/initLog - Logging session started - %s \n", Log::getDate().c_str());
        fflush(currentLog);
    }
    else
    {
        printf("Log/initLog - Log creating has failed... - %s .\n", fName.c_str());
    }
}

void Log::closeLog()
{
    fprintf(currentLog, "Log/closeLog - Logging session ended - %s \n", Log::getDate().c_str());
    fflush(currentLog);
    fclose(currentLog);
}