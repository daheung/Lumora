#pragma once

#include "Defines.h"
#include "Sockets.h"

struct FSocket;

typedef struct FSocketSubsystemState
{
	void* InternalState;
} FSocketSubsystemState;

LUMORA_C_API bool8 InitializeSocketSubsystem(FSocketSubsystemState* SocketSubsystem);

LUMORA_C_API void ReleaseSocketSubsystem(FSocketSubsystemState* SocketSubsystem);

LUMORA_C_API FSocket CreateSocket(ESocketType SocketType);

LUMORA_C_API void ReleaseSocket(FSocket* Socket);