#pragma once

typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;

typedef signed char         int8;
typedef signed short        int16;
typedef signed int          int32;
typedef signed long long    int64;

typedef float               float32;
typedef double              float64;

typedef int                 bool32;
typedef char                bool8;

typedef uint64              size_t;

#if defined(__clang__) || defined(__gcc__)
    #define STATIC_ASSERT _Static_assert
#else 
    #define STATIC_ASSERT static_assert
#endif

STATIC_ASSERT(sizeof(uint8 ) == 1, "Expected uint8 to be 1 byte");
STATIC_ASSERT(sizeof(uint16) == 2, "Expected uint16 to be 2 bytes");
STATIC_ASSERT(sizeof(uint32) == 4, "Expected uint32 to be 4 bytes");
STATIC_ASSERT(sizeof(uint64) == 8, "Expected uint64 to be 8 bytes");
        
STATIC_ASSERT(sizeof(int8 ) == 1, "Expected int8 to be 1 byte");
STATIC_ASSERT(sizeof(int16) == 2, "Expected int16 to be 2 bytes");
STATIC_ASSERT(sizeof(int32) == 4, "Expected int32 to be 4 bytes");
STATIC_ASSERT(sizeof(int64) == 8, "Expected int64 to be 8 bytes");

STATIC_ASSERT(sizeof(float32) == 4, "Expected float32 to be 4 bytes");
STATIC_ASSERT(sizeof(float64) == 8, "Expected float64 to be 8 bytes");

#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif
#ifndef NULL
    #define NULL 0
#endif
#ifndef RESTRICT
    #define RESTRICT
#endif

/** Platform-specific definitions */
#if defined(_WIN32) || defined(_WIN32) || defined(__WIN32__)
    #define PLATFORM_WINDOWS 1
    #ifndef _WIN64
        #error "64-bit Windows is not supported"
    #endif
#endif

#if defined(__linux__) || defined(__gnu_linux__)
    #define PLATFORM_LINUX 1
#endif

#if defined(__ANDROID__)
    #define PLATFORM_ANDROID 1  
#endif

#if defined(__unix__)
    #define PLATFORM_UNIX 1
#endif

#if defined(__POSIX_VERSION)
    #define PLATFORM_POSIX 1
#endif

#if __APPLE__
    #define PLATFORM_APPLE 1
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
        #define PLATFORM_IOS 1
        #define PLATFORM_IOS_SIMULATOR 1
    #elif TARGET_OS_IPHONE
        #define PLATFORM_IOS 1
    #elif TARGET_OS_MAC
    #else
        #error "Unknown Apple platform"
    #endif
#endif 

#ifdef PLATFORM_WINDOWS
    #include "Core/Platform/Windows/WindowsPlatform.h"
#endif

/* C/C++ linkage */
#ifdef __cplusplus
    #define LUMORA_EXTERN_C extern "C"
    #define LUMORA_EXTERN_C_BEGIN extern "C" {
    #define LUMORA_EXTERN_C_END }
#else
    #define LUMORA_EXTERN_C
    #define LUMORA_EXTERN_C_BEGIN
    #define LUMORA_EXTERN_C_END
#endif

#ifdef LUMORA_EXPORT
#ifdef _MSC_VER
    #define LUMORA_API __declspec(dllexport)
#else
    #define LUMORA_API __attribute__((visibility("default")))
#endif
#else
#ifdef _MSC_VER
    #define LUMORA_API __declspec(dllimport)
#else
    #define LUMORA_API
#endif
#endif

#define LUMORA_C_API LUMORA_EXTERN_C LUMORA_API
#define LUMORA_CPP_API LUMORA_API

/** Function type macros. */
#ifndef NORETURN
    #define NORETURN
#endif
#ifndef INLINE
    #define INLINE
#endif
#ifndef FORCEINLINE
    #define FORCEINLINE
#endif