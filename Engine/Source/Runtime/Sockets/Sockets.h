#pragma once

#include "Defines.h"

#include "Runtime/Sockets/SocketTypes.h"

typedef struct FSocket
{
	void* InternalData;
} FSocket;

LUMORA_C_API FSocket CreateSocket(ESocketType SocketType);

LUMORA_C_API void ReleaseSocket(FSocket* Socket);

LUMORA_C_API bool8 Bind(FSocket* Socket, FInternetAddress TargetAddress);

LUMORA_C_API bool8 Connect(FSocket* Socket, FInternetAddress TargetAddress);

LUMORA_C_API bool8 Listen(FSocket* Socket, int32 MaxBacklog);

LUMORA_C_API FSocket Accept(FSocket* Socket);

LUMORA_C_API bool8 Send(FSocket* Socket, const uint8* Data, int32 Count, int32* BytesSent);

LUMORA_C_API bool8 SendTo(FSocket* Socket, FInternetAddress TargetAddress, const uint8* Data, int32 Count, int32* BytesSent);

LUMORA_C_API bool8 Recv(FSocket* Socket, uint8* Data, int32 Count, int32* BytesRead, ESocketReceiveFlags Flags);

LUMORA_C_API bool8 Close(FSocket* Socket);

LUMORA_C_API bool8 Shutdown(FSocket* Socket, ESocketShutdownMode ShutdownMode);