#pragma once

#include <Defines.h>

/** Disable assertions by commenting out the below line. */
#define LUMORA_ASSERTIONS_ENABLED

LUMORA_C_API void ReportAssertionFailure(const char* Expression, const char* Message, const char* File, int32 Line);
LUMORA_C_API void ReportAssertionFailureFmt(const char* Expression, const char* Message, const char* File, int32 Line, ...);

#if defined(_MSC_VER)
    #include <intrin.h>

    // Q: Why is there a __nop() before __debugbreak()?
    // A: VS' debug engine has a bug where it will silently swallow explicit
    // breakpoint interrupts when single-step debugging either line-by-line or
    // over call instructions. This can hide legitimate reasons to trap. Asserts
    // for example, which can appear as if the did not fire, leaving a programmer
    // unknowingly debugging an undefined process.
    // Referenced to: Unreal Engine's PLATFORM_BREAK macro.
    #define DEBUG_BREAK() (__nop(), __debugbreak())
#elif defined(__clang__) && __has_builtin(__builtin_debugtrap)
    #define DEBUG_BREAK() __builtin_debugtrap()
#elif defined(__GNUC__) || defined(__clang__)
    #include <csignal>
    #define DEBUG_BREAK() raise(SIGTRAP)
#endif

#define LUMORA_CHECK(Expression, Message, ...)                                                     \
    do                                                                                             \
    {                                                                                              \
        if (!(Expression))                                                                         \
        {                                                                                          \
            ReportAssertionFailureFmt(#Expression, Message, __FILE__, __LINE__, ##__VA_ARGS__);    \
            DEBUG_BREAK();                                                                         \
        }                                                                                          \
    } while (0)

#ifdef LUMORA_ASSERTIONS_ENABLED
#define LUMORA_ASSERT(Expression)                                               \
    do {                                                                        \
        if (!(Expression)) {                                                    \
            ReportAssertionFailure(#Expression, "", __FILE__, __LINE__);        \
            DEBUG_BREAK();                                                      \
        }                                                                       \
    } while (0)

#define LUMORA_ASSERT_MSG(Expression, Message)                                  \
    do {                                                                        \
        if (!(Expression)) {                                                    \
            ReportAssertionFailure(#Expression, Message, __FILE__, __LINE__);   \
            DEBUG_BREAK();                                                      \
        }                                                                       \
    } while (0)

#if defined(_DEBUG) || defined(D_DEBUG)
#define LUMORA_ASSERT_DEBUG(Expression)                                         \
    do {                                                                        \
        if (!(Expression)) {                                                    \
            ReportAssertionFailure(#Expression, "", __FILE__, __LINE__);        \
            DEBUG_BREAK();                                                      \
        }                                                                       \
    } while (0)
#else
#define LUMORA_ASSERT_DEBUG(Expression)
#endif

#else
#define LUMORA_ASSERT(Expression)
#define LUMORA_ASSERT_MSG(Expression, Message)
#define LUMORA_ASSERT_DEBUG(Expression)
#endif