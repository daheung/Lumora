#include "Logger.h"
#include "Asserts.h"
#include "Platform/Platform.h"
#include "Core/Misc/CString.h"
#include "Core/Misc/FileSystem.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

typedef struct FLoggerSystemState
{
    FFileHandle LogFileHandle;
} FLoggerSystemState;

static FLoggerSystemState* GLoggerState;

static FORCEINLINE void AppendToLogFile(const char* Message)
{
    if (GLoggerState && GLoggerState->LogFileHandle.bIsValid)
    {
        /** Since the message already contains a '\n', just write the bytes directly. */
        size_t Length = Strlen(Message);
        size_t Written = 0;
        if (!FileSystemWrite(&GLoggerState->LogFileHandle, Length, Message, &Written))
        {
            PlatformConsoleWriteError("ERROR: Writing to console.log", LOG_LEVEL_ERROR);
        }
    }
}

bool8 InitializeLogging(size_t* const MemoryRequirement, void* State)
{
    *MemoryRequirement = sizeof(FLoggerSystemState);
    if (GLoggerState == NULL)
    {
        return TRUE;
    }

    GLoggerState = State;

    /** Create new/wipe existing log file, then open it, */
    if (!FileSystemOpen("console.log", FILE_MODE_WRITE, FALSE, &GLoggerState->LogFileHandle))
    {
        PlatformConsoleWriteError("ERROR: Unable to open console.log for writing.", LOG_LEVEL_ERROR);
        return FALSE;
    }

    /** TODO: Create log file. */
    return TRUE;
}

void ShutdownLogging()
{
    GLoggerState = NULL;
    /** TODO: Cleanup logging/write queued entries.*/
}

void ReportAssertionFailure(const char* Expression, const char* Message, const char* File, int32 Line)
{
    LogOutput(LOG_LEVEL_FATAL, "Assertion failure: %s, Message: %s, File: %s, Line: %d\n", Expression, Message, File, Line);
}

LUMORA_C_API void ReportAssertionFailureFmt(const char* Expression, const char* Message, const char* File, int32 Line, ...)
{
    char FormattedMessage[4096];

    va_list Args;
    va_start(Args, Line);
    GetVarArgs(FormattedMessage, sizeof(FormattedMessage), Message, Args);
    va_end(Args);

    LogOutput(LOG_LEVEL_FATAL, "Assertion failure: %s, Message: %s, File: %s, Line: %d", Expression, FormattedMessage, File, Line);
}

void LogOutput(ELogLevel LogLevel, const char* Message, ...)
{
    /**
     * TODO: These string operations are all pretty slow. This needs to be 
     * moved to another thread eventually, along with the file writes, to
     * avoid slowing things down while the engine is trying to run.
     */
    const char* LogLevelStrings[6] = { "[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: " };
    const bool8 bIsError =
        LogLevel == LOG_LEVEL_FATAL ||
        LogLevel == LOG_LEVEL_ERROR;

    /** 
     * Technically imposes a 64k character limit on a single log entry, but ...
     * DON't DO THAT!
     */
    char OutMessage[65536] = { 0 };

    const int32 Offset = FormatString(OutMessage, sizeof(OutMessage), "%s", LogLevelStrings[LogLevel]);
    /** 
     * Format original message.
     * NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a "typedef char* va_list" in some
     * cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
     * which is the type GCC/Clang's va_start expects.
     */
    va_list Args;
    va_start(Args, Message);
    StringFormatV(OutMessage + Offset, sizeof(OutMessage) - Offset, Message, Args);
    va_end(Args);

    size_t Length = Strlen(OutMessage);
    if (Length + 1 < sizeof(OutMessage))
    {
        OutMessage[Length] = '\n';
        OutMessage[Length + 1] = '\0';
    }

    /** platform-specific output. */
    // printf("%s", OutMessage2);
    if (bIsError)
    {
        PlatformConsoleWriteError(OutMessage, LogLevel);
    } 
    else
    {
        PlatformConsoleWrite(OutMessage, LogLevel);
    }

    /** Queue a copy to be written to the log file. */
    AppendToLogFile(OutMessage);
}

