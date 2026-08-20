#include "Core/HAL/Thread.h"
#include "Core/Platform/Platform.h"
#include "Core/Logger.h"

#if PLATFORM_WINDOWS
#include <Windows.h>

LUMORA_C_API bool8 HThreadCreate(FThreadStartFunc ThreadStartFunc, void* Params, bool8 bAutoDetach, FThread* OutThread)
{
	if (!ThreadStartFunc)
	{
		return FALSE;
	}

	HANDLE Handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadStartFunc, Params, 0, (DWORD*)&OutThread->ThreadId);
	LUMORA_DEBUG("Starting process on thread id: %#x", OutThread->ThreadId);
	if (!Handle)
	{
		return FALSE;
	}

	if (bAutoDetach)
	{
		CloseHandle(Handle);
	}

	return TRUE;
}

LUMORA_C_API void HThreadRelease(FThread* Thread)
{
	if (Thread && Thread->InternalData)
	{
		DWORD ExitCode = 0;
		bool8 bSuccessExit = GetExitCodeThread(Thread->InternalData, &ExitCode);
		//if (ExitCode == STILL_ACTIVE)
		//{
		//	TerminateThread(Thread->InternalData, 0); // 0 = failure
		//}
		CloseHandle((HANDLE)Thread->InternalData);
		Thread->InternalData = NULL;
		Thread->ThreadId = 0;
	}
}

LUMORA_C_API void HThreadDetach(FThread* Thread)
{
	if (Thread && Thread->InternalData)
	{
		CloseHandle(Thread->InternalData);
		Thread->InternalData = NULL;
	}
}

LUMORA_C_API void HThreadCancel(FThread* Thread)
{
	if (Thread && Thread->InternalData)
	{
		TerminateThread((HANDLE)Thread->InternalData, 0);
		Thread->InternalData = NULL;
	}
}

LUMORA_C_API bool8 HThreadIsActive(FThread* Thread)
{
	if (Thread && Thread->InternalData)
	{
		DWORD ExitCode = WaitForSingleObject(Thread->InternalData, 0);
		if (ExitCode == WAIT_TIMEOUT)
		{
			return TRUE;
		}
	}

	return FALSE;
}

LUMORA_C_API void HThreadSleep(FThread* Thread, uint64 MilliSecond)
{
	LUMORA_UNUSED_PARAM(Thread);
	PlatformSleep(MilliSecond);
}

LUMORA_C_API uint64 HThreadGetId(void)
{
	return (uint64)GetCurrentThreadId();
}

#endif
