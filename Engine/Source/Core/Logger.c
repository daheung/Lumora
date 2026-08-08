#include "Logger.h"
#include "Asserts.h"
#include "Platform/Platform.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

void ReportAssertionFailure(const char* Expression, const char* Message, const char* File, int32 Line)
{
    LogOutput(LOG_LEVEL_FATAL, "Assertion failure: %s, Message: %s, File: %s, Line: %d\n", Expression, Message, File, Line);
}

bool8 InitializeLogging()
{
    /** TODO: Create log file. */

    return TRUE;
}

void ShutdownLogging()
{
    /** TODO: Cleanup logging/write queued entries.*/
}

void LogOutput(ELogLevel LogLevel, const char* Message, ...)
{
    const char* LogLevelStrings[6] = { "[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: " };
    const bool8 bIsError =
        LogLevel == LOG_LEVEL_FATAL ||
        LogLevel == LOG_LEVEL_ERROR;

    /** 
     * Technically imposes a 64k character limit on a single log entry, but ...
     * DON't DO THAT!
     */
    const int32 MessageLength = 65536;
    char OutMessage[MessageLength];
    memset(OutMessage, 0, sizeof(OutMessage));

    /** 
     * Format original message.
     * NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a "typedef char* va_list" in some
     * cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
     * which is the type GCC/Clang's va_start expects.
     */
    __builtin_va_list Args;
    va_start(Args, Message);
    vsprintf(OutMessage, Message, Args);
    va_end(Args);

    char OutMessage2[65536];
    sprintf(OutMessage2, "%s%s\n", LogLevelStrings[LogLevel], OutMessage);

    /** platform-specific output. */
    // printf("%s", OutMessage2);
    if (bIsError)
    {
        PlatformConsoleWriteError(OutMessage2, LogLevel);
    } 
    else
    {
        PlatformConsoleWrite(OutMessage2, LogLevel);
    }
}

