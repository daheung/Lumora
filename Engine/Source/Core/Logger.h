#pragma once

#include "Defines.h"

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

// Disable debug and trace logging for release builds.
#if LRELEASE == 1
    #define LOG_DEBUG_ENABLED 0
    #define LOG_TRACE_ENABLED 0
#endif

typedef enum ELogLevel
{
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN  = 2,
    LOG_LEVEL_INFO  = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5,

    LOG_LEVEL_COUNT,
} ELogLevel;

LUMORA_C_API bool8 InitializeLogging();
LUMORA_C_API void ShutdownLogging();

LUMORA_C_API void LogOutput(ELogLevel LogLevel, const char* Message, ...);

#define LUMORA_FATAL(Message, ...) LogOutput(LOG_LEVEL_FATAL, Message, ##__VA_ARGS__);

#ifndef LUMORA_ERROR
    /** Logs an error-level message */
    #define LUMORA_ERROR(Message, ...) LogOutput(LOG_LEVEL_ERROR, Message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED
    /** Logs a warning-level message */
    #define LUMORA_WARN(Message, ...) LogOutput(LOG_LEVEL_WARN, Message, ##__VA_ARGS__);
#else
    #define LUMORA_WARN(Message, ...)
#endif

#if LOG_INFO_ENABLED
    /** Logs an info-level message */
    #define LUMORA_INFO(Message, ...) LogOutput(LOG_LEVEL_INFO, Message, ##__VA_ARGS__);
#else
    #define LUMORA_INFO(Message, ...)
#endif

#if LOG_DEBUG_ENABLED
    /** Logs a debug-level message */
    #define LUMORA_DEBUG(Message, ...) LogOutput(LOG_LEVEL_DEBUG, Message, ##__VA_ARGS__);
#else
    #define LUMORA_DEBUG(Message, ...)
#endif

#if LOG_TRACE_ENABLED
    /** Logs a trace-level message */
    #define LUMORA_TRACE(Message, ...) LogOutput(LOG_LEVEL_TRACE, Message, ##__VA_ARGS__);
#else
    #define LUMORA_TRACE(Message, ...)
#endif