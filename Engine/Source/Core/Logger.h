#pragma once

#include "Defines.h"
#include "Asserts.h"

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

/**
 * @brief Initializes logging system. Call twice; once with state = 0 to get required memory size,
 * then a second time passing allocated memory to state.
 * 
 * @param MemoryRequirement A pointer to hold the required size of internal state.
 * @param State 0 if just requesting memory requirement, otherwise allocated block of memory.
 * @return bool8 True on success; otherwise false.
 */
LUMORA_C_API bool8 InitializeLogging(size_t* const MemoryRequirement, void* State);

LUMORA_C_API void ShutdownLogging();

LUMORA_C_API void LogOutput(ELogLevel LogLevel, const char* Message, ...);

#define LUMORA_LOG(Expression, LogLevel, Message, ...)          \
    do {                                                        \
        if (!(Expression)) {                                    \
            LogOutput(LogLevel, Message, ##__VA_ARGS__);        \
            DEBUG_BREAK();                                      \
        }                                                       \
    } while(0)

#define LUMORA_FATAL(Message, ...) LUMORA_LOG(0, LOG_LEVEL_FATAL, Message, ##__VA_ARGS__);

#ifndef LUMORA_ERROR
    /** Logs an error-level message */
    #define LUMORA_ERROR(Message, ...) LUMORA_LOG(0, LOG_LEVEL_ERROR, Message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED
    /** Logs a warning-level message */
    #define LUMORA_WARN(Message, ...) LogOutput(LOG_LEVEL_ERROR, Message, ##__VA_ARGS__);
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