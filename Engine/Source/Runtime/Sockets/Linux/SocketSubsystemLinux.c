#include "Runtime/Sockets/SocketSubsystem.h"

#if defined(PLATFORM_LINUX)

#include "Core/HAL/LumoraMemory.h"
#include "Core/Logger.h"
#include "Runtime/Sockets/Linux/SocketLinux.h"

typedef struct FInternalState
{
	bool8 bInitialized;
} FInternalState;

LUMORA_C_API bool8 InitializeSocketSubsystem(FSocketSubsystemState* SocketSubsystem)
{
	FInternalState* InternalState = HAllocate(sizeof(FInternalState), MEMORY_TAG_APPLICATION);

	InternalState->bInitialized = TRUE;
	SocketSubsystem->InternalState = InternalState;

	return TRUE;
}

LUMORA_C_API void ReleaseSocketSubsystem(FSocketSubsystemState* SocketSubsystem)
{
	FInternalState* InternalState = (FInternalState*)SocketSubsystem->InternalState;
	InternalState->bInitialized = FALSE;
	return;
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