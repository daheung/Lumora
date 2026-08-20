#include "Core/HAL/Mutex.h"
#include "Core/Platform/Platform.h"
#include "Core/Logger.h"

#if PLATFORM_WINDOWS

#include <Windows.h>

LUMORA_C_API bool8 HMutexCreate(HMutex* OutMutex)
{
	if (!OutMutex)
	{
		return FALSE;
	}

	OutMutex->InternalData = CreateMutex(NULL, FALSE, NULL);
	if (!OutMutex->InternalData)
	{
		LUMORA_ERROR("Unable to create mutex.");
		return FALSE;
	}

	LUMORA_TRACE("Created mutex.");
	return TRUE;
}

LUMORA_C_API void HMutexRelease(HMutex* Mutex)
{
	if (Mutex && Mutex->InternalData)
	{
		CloseHandle(Mutex->InternalData);
		//LUMORA_TRACE("Destroyed mutex.");
		Mutex->InternalData = NULL;
	}
}

LUMORA_C_API bool8 HMutexLock(HMutex* Mutex)
{
	if (!Mutex)
	{
		return FALSE;
	}

	DWORD Result = WaitForSingleObject((HANDLE)Mutex->InternalData, INFINITE);
	switch (Result)
	{
	/** The thread got ownership of the mutex. */
	case WAIT_OBJECT_0:
		//LUMORA_TRACE("Mutex locked.");
		return TRUE;

	/** The thread got ownership of the abandoned mutex. */
	case WAIT_ABANDONED:
		LUMORA_ERROR("Mutex lock failed.");
		return FALSE;
	}

	//LUMORA_TRACE("Mutex locked.");
	return TRUE;
}

LUMORA_C_API bool8 HMutexUnlock(HMutex* Mutex)
{
	if (!(Mutex && Mutex->InternalData))
	{
		return FALSE;
	}

	bool8 bReleaseSuccess = ReleaseMutex((HANDLE)Mutex->InternalData);
	LUMORA_TRACE("Mutex unlocked.");

	return bReleaseSuccess != 0;
}

#endif