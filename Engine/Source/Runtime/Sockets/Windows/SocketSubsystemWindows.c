#include "Runtime/Sockets/SocketSubsystem.h"

#if PLATFORM_WINDOWS

#include "Core/HAL/LumoraMemory.h"
#include "Core/Logger.h"
#include "Runtime/Sockets/Windows/SocketWindows.h"

#include <winsock2.h>
#include <Windows.h>


typedef struct FInternalState
{
	bool8 bInitialized;

	WSADATA WindowsSubsystem;

	/** TODO: Custom critical section. */
	CRITICAL_SECTION CriticalSection;
} FInternalState;

LUMORA_C_API bool8 InitializeSocketSubsystem(FSocketSubsystemState* SocketSubsystem)
{
	FInternalState* InternalState = HAllocate(sizeof(FInternalState), MEMORY_TAG_APPLICATION);

	/*
	 * Critical section is not required at this stage.
	 * It is added in advance to ensure thread safety when this module
	 * is migrated to C++ and implemented using the Singleton pattern.
	 */
	InitializeCriticalSection(&InternalState->CriticalSection);

	/** Initialize Window socket */
	const bool8 bInitWinsockSucceed = WSAStartup(MAKEWORD(2, 2), &InternalState->WindowsSubsystem) == 0;
	if (!bInitWinsockSucceed)
	{
		LUMORA_ERROR("Initialize Window socket failed.");
		return FALSE;
	}

	InternalState->bInitialized = TRUE;
	SocketSubsystem->InternalState = InternalState;

	return TRUE;
}

LUMORA_C_API void ReleaseSocketSubsystem(FSocketSubsystemState* SocketSubsystem)
{
	if (!SocketSubsystem)
	{
		LUMORA_ERROR("Cannot release SocketSubsystem; SocketSubsysten is null.");
		return;
	}

	const bool8 bInitWinsockSucceed = WSACleanup() != SOCKET_ERROR;
	if (!bInitWinsockSucceed)
	{
		LUMORA_ERROR("Initialize Window socket failed.");
		return;
	}
}

LUMORA_C_API FSocket CreateSocket(ESocketType SocketType)
{
	return CreateSocketImpl(SocketType);
}

LUMORA_C_API void ReleaseSocket(FSocket* Socket)
{
	ReleaseSocketImpl(Socket);
}

#endif