#pragma once

#include "Defines.h"

typedef struct FPlatformState{
    void* InternalState;
} FPlatformState;

LUMORA_C_API bool8 PlatformStartup(FPlatformState* PlatformState, const char* ApplicationName, int32 X, int32 Y, int32 Width, int32 Height);
LUMORA_C_API void PlatformShutdown(FPlatformState* PlatformState);

LUMORA_C_API bool8 PlatformPumpMessage(FPlatformState* PlatformState);

LUMORA_C_API void* PlatformAllocate(uint64 AllocSize, bool8 bAligned);
LUMORA_C_API void  PlatformFree(void* Block, bool8 bAligned);
LUMORA_C_API void* PlatformZeroMemory(void* Block, uint64 AllocSize);
LUMORA_C_API void* PlatformCopyMemory(void* Dest, const void* Src, uint64 AllocSize);
LUMORA_C_API void* PlatformSetMemory(void* Dest, int32 Value, uint64 AllocSize);

LUMORA_C_API void PlatformConsoleWrite(const char* Message, uint8 Color);
LUMORA_C_API void PlatformConsoleWriteError(const char* Message, uint8 Color);

LUMORA_C_API float64 PlatformGetAbsoluteTime(void);

/**
 * Sleep on the thread for the provided ms, This blocks the main thread.
 * Should only be used for giving time back to the OS for unused update power.
 * Therefore it is not exported.
 */
LUMORA_C_API void PlatformSleep(uint64 MilliSecond);