#include "Core/Platform/Platform.h"

/** Linux platform layer. */
#if PLATFORM_LINUX
#define _POSIX_C_SOURCE 200809L

#include "Core/HAL/Mutex.h"
#include "Core/Logger.h"

#include <pthread.h>
#include <errno.h>          // For error reporting
#include <sys/sysinfo.h>    // Processor info

LUMORA_C_API bool8 HMutexCreate(HMutex* OutMutex)
{
    if (!OutMutex)
    {
        return FALSE;
    }

    /** Initiazlie Mutex */
    pthread_mutex_t Mutex;
    int32 Result = pthread_mutex_init(&Mutex, 0);
    if (Result != 0)
    {
        LUMORA_ERROR("Mutex creation failure.");
        return FALSE;
    }

    /** Save off the mutex handle. */
    OutMutex->InternalData = PlatformAllocate(sizeof(pthread_mutex_t), FALSE);
    *(pthread_mutex_t*)OutMutex->InternalData = Mutex;

    return TRUE;
}

LUMORA_C_API void HMutexRelease(HMutex* Mutex)
{
    if (Mutex)
    {
        int32 Result = pthread_mutex_destroy((pthread_mutex_t*)Mutex->InternalData);
        switch (Result)
        {
        case 0:
            // LUMORA_TRACE("Mutex destroyed.");
            break;
        case EBUSY:
            LUMORA_ERROR("Unable to destroy mutex: mutex is locked or referenced.");
            break;
        case EINVAL:
            LUMORA_ERROR("Unable to destroy mutex: the value specified by mutex is invalid.");
            break;
        default:
            LUMORA_ERROR("An handled error has occurred while destroy a mutex: errno=%i", Result);
            break;
        }

        PlatformFree(Mutex->InternalData, FALSE);
        Mutex->InternalData = NULL;
    }
}

LUMORA_C_API bool8 HMutexLock(HMutex* Mutex)
{
    if  (!Mutex)
    {
        return FALSE;
    }

    /** Lock mutex */
    int32 Result = pthread_mutex_lock((pthread_mutex_t*)Mutex->InternalData);
    switch (Result)
    {
    case 0:
        /** Success, everything else is a failure. */
        // LUMORA_TRACE("Obained mutex lock.");
        return TRUE;
    case EOWNERDEAD:
        LUMORA_ERROR("Owning thread terminated while mutex still active.");
        return FALSE;
    case EAGAIN:
        LUMORA_ERROR("Unable to obtain mutex lock: the maximum number of recursive mutex locks has been reached.");
        return FALSE;
    case EBUSY:
        LUMORA_ERROR("Unable to obtain mutex lock: a mutex lock already exists.");
        return FALSE;
    case EDEADLK:
        LUMORA_ERROR("Unable to obtain mutex lock: a mutex deadlock wad detected.");
        return FALSE;
    default:
        LUMORA_ERROR("An handled error has occurred while obtainint a mutex lock: errno=%i", Result);
        return FALSE;
    }
}

LUMORA_C_API bool8 HMutexUnlock(HMutex* Mutex)
{
    if (!Mutex)
    {
        return FALSE;
    }

    if (Mutex->InternalData)
    {
        int32 Result = pthread_mutex_unlock((pthread_mutex_t*)Mutex->InternalData);
        switch (Result)
        {
        case 0:
            LUMORA_TRACE("Freed mutex lock.");
            return TRUE;
        case EOWNERDEAD:
            LUMORA_ERROR("Unable to unlock mutex: owning thread terminated while mutex still active.");
            return FALSE;
        case EPERM:
            LUMORA_ERROR("Unable to unlock mutex: mutex not owned by current thread.");
            return FALSE;
        default:
            LUMORA_ERROR("An handled error has occurred while obtainint a mutex lock: errno=%i", Result);
            return FALSE;
        }
    }

    return FALSE;
}

#endif