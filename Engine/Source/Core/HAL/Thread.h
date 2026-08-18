#pragma once

#include "Defines.h"

/**
 * Represents a process thread in the system to be used for work.
 * Generally should not be created directly in user code.
 * This calls to the playform-specific thread implementation.
 */
typedef struct FThread
{
	void* InternalData;
	uint64 ThreadId;
} FThread;

/** A function pointer to be invoked when the thread starts. */
typedef uint32 (*FThreadStartFunc)(void*);

/**
 * Creates a new thread. immediately calling the function pointed to.
 * @param ThreadStartFunc The pointer to the function to be invoked immediately. Required,
 * @param Params A pointer to any data to be passed to the ThreadStartFunc. Optional. Pass 0/NULL if not used.
 * @param bAutoDetach Indicates if the thread should immediately release its resources when the work is complete. If true, OutThread is not set.
 * @param OutThread A pointer to hold the created thread. if bAutoDetach is false;
 * @returns true if successfully created; otherwise false.
 */
LUMORA_C_API bool8 HThreadCreate(FThreadStartFunc ThreadStartFunc, void* Params, bool8 bAutoDetach, FThread* OutThread);

/** Destroys the given thread. */
LUMORA_C_API void HThreadRelease(FThread* Thread);

/** Detaches the thread, automatically releasing resources when work is complete. */
LUMORA_C_API void HThreadDetach(FThread* Thread);

/** Cancels work on thr thread, if possible, and releases resources when possible. */
LUMORA_C_API void HThreadCancel(FThread* Thread);

/**
 * Indicates if the thread is currently active.
 * @returns True if active; otherwise false.
 */
LUMORA_C_API bool8 HThreadIsActive(FThread* Thread);

/**
 * Sleeps on the given thread for a given number of milliseconds. Should be called from the
 * thread requiring the sleep.
 */
LUMORA_C_API void HThreadSleep(FThread* Thread, uint64 MilliSecond);

LUMORA_C_API uint64 HThreadGetId(void);