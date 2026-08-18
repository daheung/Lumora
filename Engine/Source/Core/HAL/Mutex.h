#pragma once

#include "Defines.h"

/**
 * A mutex to be used for synchronization purposes. A mutex (or
 * mutual exclusion) is used to limit access to a resource when 
 * there are multiple threads of execution around that resource.
 */
typedef struct HMutex
{
	void* InternalData;
} HMutex;

/**
 * Creates a mutex.
 * @param OutMutex A pointer to hold the created mutex.
 * @returns True if created successfully; otherwise false.
 */
LUMORA_C_API bool8 HMutexCreate(HMutex* OutMutex);

/**
 * @brief Destroys the provided mutex.
 * 
 * @param Mutex A pointer to the mutex to be destroyed.
 */
LUMORA_C_API void HMutexRelease(HMutex* Mutex);

/**
 * Creates a mutex lock.
 * @param Mutex A pointer to the mutex.
 * @returns True if locked successfully; otherwise false.
 */
LUMORA_C_API bool8 HMutexLock(HMutex* Mutex);

/**
 * Unlocks the given mutex.
 * @param Mutex The mutex to unlock
 * @returns True if unlocked successfully; otherwise false.
 */
LUMORA_C_API bool8 HMutexUnlock(HMutex* Mutex);