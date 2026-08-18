#include "Core/Platform/Platform.h"

/** Linux platform layer. */
#if PLATFORM_LINUX
#define _POSIX_C_SOURCE 200809L

#include "Core/HAL/Mutex.h"
#include "Core/HAL/Thread.h"
#include "Core/Logger.h"

#include <pthread.h>
#include <errno.h>          // For error reporting
#include <sys/sysinfo.h>    // Processor info

/** TODO: Replace the void* return type with an integer or otherwise. */
typedef void* (*FLinuxThreadFuncType)(void*);

LUMORA_C_API bool8 HThreadCreate(FThreadStartFunc ThreadStartFunc, void* Params, bool8 bAutoDetach, FThread* OutThread)
{
    if (!ThreadStartFunc)
    {
        return FALSE;
    }

    /** pthread_create uses a function pointer that returns void*, so cold-cast to this type. */
    int32 Result = pthread_create((pthread_t*)&OutThread->ThreadId, 0, (FLinuxThreadFuncType)ThreadStartFunc, Params);
    if (Result != 0)
    {
        switch (Result)
        {
        case EAGAIN:
            LUMORA_ERROR("Failed to create thread: insufficient resources to create another thread.");
            return FALSE;
        case EINVAL:
            LUMORA_ERROR("Failed to create thread: invalid settings were passed in attributes.");
            return FALSE;
        default:
            LUMORA_ERROR("Failed to create thread: an unhandled error has occurred. errno=%i", Result);
            return FALSE;
        }
    }

    LUMORA_DEBUG("Starting process n thread id : %#x", OutThread->ThreadId);

    /** Only save off the handle if not auto-detaching. */
    if (!bAutoDetach)
    {
        OutThread->InternalData = PlatformAllocate(sizeof(uint64), FALSE);
        *(uint64*)OutThread->InternalData = OutThread->ThreadId;
    }
    else
    {
        /** If immeidately detaching, make sure the operation is a success. */
        Result = pthread_detach(OutThread->ThreadId);
        if (Result != 0)
        {
            switch (Result)
            {
            case EINVAL:
                LUMORA_ERROR("Failed to detach newly-created thread: thread is not a joinable thread.");
                return FALSE;
            case ESRCH:
                LUMORA_ERROR("Failed to detach newly-created thread: no thread with the id %#x could be found.", OutThread->ThreadId);
                return FALSE;
            default:
                LUMORA_ERROR("Failed to detach newly-created thread: an unknown error has occurred. errno=%i", Result);
                return FALSE;
            }
        }
    }

    return TRUE;
}

LUMORA_C_API void HThreadRelease(FThread* Thread)
{
    HThreadCancel(Thread);
}

LUMORA_C_API void HThreadDetach(FThread* Thread)
{
    if (Thread->InternalData)
    {
        int32 Result = pthread_detach(*(pthread_t*)Thread->InternalData);
        if (Result != 0)
        {
            switch (Result)
            {
            case EINVAL:
                LUMORA_ERROR("Failed to detach newly-created thread: thread is not a joinable thread.");
                break;
            case ESRCH:
                LUMORA_ERROR("Failed to detach newly-created thread: no thread with the id %#x could be found.", Thread->ThreadId);
                break;
            default:
                LUMORA_ERROR("Failed to detach newly-created thread: an unknown error has occurred. errno=%i", Result);
                break;
            }
        }

        PlatformFree(Thread->InternalData, FALSE);
        Thread->ThreadId = (uint64)0;
    }
}

LUMORA_C_API void HThreadCancel(FThread* Thread)
{
    if (Thread->InternalData)
    {
        int32 Result = pthread_cancel(*(pthread_t*)Thread->InternalData);
        if (Result != 0)
        {
            switch (Result)
            {
            case ESRCH:
                LUMORA_ERROR("Failed to cancel thread: no thread with the id %#x could be found.", Thread->ThreadId);
                break;
            default:
                LUMORA_ERROR("Failed to cancel thread: an unknown error has occurred. errno=%i", Result);
                break;
            }
        }

        PlatformFree(Thread->InternalData, FALSE);
        Thread->InternalData = NULL;
        Thread->ThreadId = (uint64)0;
    }
}

LUMORA_C_API bool8 HThreadIsActive(FThread* Thread)
{
    /** TODO: Find a better way to verifi this. */
    return Thread->InternalData != 0;
}

LUMORA_C_API void HThreadSleep(FThread* Thread, uint64 MilliSecond)
{
    LUMORA_UNUSED_PARAM(Thread);
    PlatformSleep(MilliSecond);
}

LUMORA_C_API uint64 HThreadGetId(void)
{
    return (uint64)pthread_self();
}

#endif