#pragma once

#include "Defines.h"

typedef struct FPlatformState{
    void* InternalState;
} FPlatformState;

bool8 PlatformStartup(FPlatformState* PlatformState, const char* ApplicationName, int32 X, int32 Y, int32 Width, int32 Height);
void PlatformShutdown(FPlatformState* PlatformState);

bool8 PlatformPumpMessage(FPlatformState* PlatformState);

void* PlatformAllocate(uint64 AllocSize, bool8 bAligned);
void  PlatformFree(void* Block, bool8 bAligned);
void* PlatformZeroMemory(void* Block, uint64 AllocSize);
void* PlatformCopyMemory(void* Dest, const void* Src, uint64 AllocSize);
void* PlatformSetMemory(void* Dest, int32 Value, uint64 AllocSize);

void PlatformConsoleWrite(const char* Message, uint8 Color);
void PlatformConsoleWriteError(const char* Message, uint8 Color);

float64 PlatformGetAbsoluteTime(void);

/**
 * Sleep on the thread for the provided ms, This blocks the main thread.
 * Should only be used for giving time back to the OS for unused update power.
 * Therefore it is not exported.
 */
void PlatformSleep(uint64 MilliSecond);


/**
 * @brief Obtains the number of processor cores.
 * 
 * @returns The number of processor cores.
 */
LUMORA_C_API int32 PlatformGetProcessorCount(void);