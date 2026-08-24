#include "SocketLinux.h"

#if defined(PLATFORM_LINUX)

#include "Runtime/Sockets/SocketTypes.h"

#include "Core/HAL/LumoraMemory.h"
#include "Core/Misc/CString.h"
#include "Core/Logger.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef struct FInternalSocketState
{
	int32 Socket;

	FInternetAddress LocalAddress;
	FInternetAddress RemoteAddress;
	
	ESocketShutdownMode ShutdownMode;
	ESocketType SocketType;

	bool8 bSocketInitialized;
	bool8 bSocketClosed;
	bool8 bSocketShutdown;
} FInternalSocketState;

static FORCEINLINE int32 TranslateRecvFlags(ESocketReceiveFlags Flags);
static FORCEINLINE int32 TranslateShutdownMode(ESocketShutdownMode Flags);
static FORCEINLINE bool8 GetLocalAddressBySocket(const FSocket* Socket, FInternetAddress* OutInternetAddress);

FSocket CreateSocketImpl(ESocketType SocketType)
{
	LUMORA_ASSERT_MSG(SocketType != SOCKET_TYPE_UNKNOWN, "SocketType is Unknown");

	/** Create socket */
	int LnxSocket = -1;
	switch (SocketType)
	{
	case SOCKET_TYPE_DATAGRAM:
		LnxSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		break;
	case SOCKET_TYPE_STREAMING:
		LnxSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		break;
	default:
		LUMORA_ERROR("Failed to create socket.");
		return (FSocket) { NULL };
	}
	
	if (LnxSocket == -1)
	{
		LUMORA_ERROR("Failed to create socket.");
		return (FSocket){ NULL };
	}

	FInternalSocketState* SocketState = HAllocate(sizeof(FInternalSocketState), MEMORY_TAG_APPLICATION);
	HZeroMemory(SocketState, sizeof(FInternalSocketState));
	SocketState->Socket = LnxSocket;
	SocketState->SocketType = SocketType;
	SocketState->bSocketInitialized = TRUE;

	return (FSocket){ .InternalData = SocketState };
}

void ReleaseSocketImpl(FSocket* Socket)
{
	LUMORA_ASSERT_MSG(Socket, "Socket is null.");
	LUMORA_ASSERT_MSG(Socket->InternalData, "Socket internal data is null.");

	FInternalSocketState* SocketState = (FInternalSocketState*)Socket->InternalData;
	if (!SocketState->bSocketInitialized)
	{
		LUMORA_WARN("Socket is not initialized; initialize the socket before called ReleaseSocket().");
		return;
	}

	if (!SocketState->bSocketClosed)
	{
		LUMORA_WARN("Socket is not closed; close the socket before called ReleaseSocket().");
		return;
	}

	HFree(SocketState, sizeof(FInternalSocketState), MEMORY_TAG_APPLICATION);
}

LUMORA_C_API bool8 Bind(FSocket* Socket, FInternetAddress BindAddress)
{
	LUMORA_ASSERT_MSG(Socket, "Socket is null.");
	LUMORA_ASSERT_MSG(Socket->InternalData, "Socket internal data is null.");

	uint32 Ip = 0;
	int32 Result = inet_pton(AF_INET, BindAddress.Ip, (void*)&Ip);
	if (Result != 1)
	{
		LUMORA_WARN("Bind socket failed; Invalid Ip.");
		return FALSE;
	}

	FInternalSocketState* SocketState = ((FInternalSocketState*)Socket->InternalData);

	struct sockaddr_in SvrAddr = { 0 };
	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_port = htons(BindAddress.Port);
	SvrAddr.sin_addr.s_addr = Ip;

	int32 LnxSocket = SocketState->Socket;
	Result = bind(LnxSocket, (const struct sockaddr*)&SvrAddr, sizeof(struct sockaddr_in));
	if (Result != -1)
	{
		GetLocalAddressBySocket(Socket, &SocketState->LocalAddress);
		return TRUE;
	}

	return FALSE;
}

LUMORA_C_API bool8 Connect(FSocket* Socket, FInternetAddress DestAddress)
{
	LUMORA_ASSERT_MSG(Socket, "Socket is null.");
	LUMORA_ASSERT_MSG(Socket->InternalData, "Socket internal data is null.");

	uint32 Ip = 0;
	int32 Result = inet_pton(AF_INET, DestAddress.Ip, (void*)&Ip);
	if (Result != 1)
	{
		LUMORA_WARN("Connect socket failed; Invalid Ip.");
		return FALSE;
	}

	FInternalSocketState* ClientSocketState = ((FInternalSocketState*)Socket->InternalData);

	struct sockaddr_in TargetAddr = { 0 };
	TargetAddr.sin_family = AF_INET;
	TargetAddr.sin_port = htons(DestAddress.Port);
	TargetAddr.sin_addr.s_addr = Ip;

	int32 WinSocket = ClientSocketState->Socket;
	const int32 ConnectResult = connect(WinSocket, (const struct sockaddr*)&TargetAddr, sizeof(struct sockaddr_in));
	if (ConnectResult != -1)
	{
		ClientSocketState->RemoteAddress = DestAddress;
		GetLocalAddressBySocket(Socket, &ClientSocketState->LocalAddress);
		return TRUE;
	}

	return FALSE;
}

LUMORA_C_API bool8 Listen(FSocket* Socket, int32 MaxBacklog)
{
	LUMORA_ASSERT_MSG(Socket, "Socket is null.");
	LUMORA_ASSERT_MSG(Socket->InternalData, "Socket internal data is null.");
	int32 LnxSocket = ((FInternalSocketState*)Socket->InternalData)->Socket;
	return listen(LnxSocket, MaxBacklog) != -1;
}

LUMORA_C_API FSocket Accept(FSocket* Socket)
{
	LUMORA_ASSERT_MSG(Socket, "Socket is null.");
	LUMORA_ASSERT_MSG(Socket->InternalData, "Socket internal data is null.");

	const FInternalSocketState* ServerSocketState = ((FInternalSocketState*)Socket->InternalData);
	
	struct sockaddr_in ClientAddr = { 0 };
	socklen_t ClientAddrLen = sizeof(ClientAddr);
	int32 ClientLnxSocket = accept(ServerSocketState->Socket, (struct sockaddr*)&ClientAddr, &ClientAddrLen);
	if (ClientLnxSocket != -1)
	{
		FSocket OutSocket = { 0 };
		FInternalSocketState* ClientSocketState = HAllocate(sizeof(FInternalSocketState), MEMORY_TAG_APPLICATION);
		HZeroMemory(ClientSocketState, sizeof(FInternalSocketState));
		ClientSocketState->Socket			= ClientLnxSocket;
		ClientSocketState->bSocketClosed	= FALSE;
		ClientSocketState->bSocketShutdown	= FALSE;
		ClientSocketState->ShutdownMode		= SOCKET_SHUTDOWN_MODE_KNOWN;
		ClientSocketState->SocketType		= ServerSocketState->SocketType;

		FInternetAddress ClientSrcAddress = { 0 };
		inet_ntop(AF_INET, &ClientAddr.sin_addr, ClientSrcAddress.Ip, INET_ADDRSTRLEN);
		ClientSrcAddress.Port = ntohs(ClientAddr.sin_port);

		ClientSocketState->LocalAddress		= ServerSocketState->LocalAddress;
		ClientSocketState->RemoteAddress	= ClientSrcAddress;
		OutSocket.InternalData = ClientSocketState;

		return OutSocket;
	}

	return (FSocket){ NULL };
}

LUMORA_C_API bool8 Send(FSocket* Socket, const uint8* Data, int32 Count, int32* BytesSent)
{
	LUMORA_ASSERT_MSG(Socket, "Socket is null.");
	LUMORA_ASSERT_MSG(Socket->InternalData, "Socket internal data is null.");

	if (BytesSent)
	{
		*BytesSent = 0;
	}

	if (Data == NULL)
	{
		LUMORA_WARN("Socket Send failed. Buffer data to send is null.");
		return FALSE;
	}
	if (Count < 0)
	{
		LUMORA_WARN("Socket Send failed. Invalid data size.");
		return FALSE;
	}

	int32 LnxSocket = ((FInternalSocketState*)Socket->InternalData)->Socket;
	int32 BytesSentResult = send(LnxSocket, (const char*)Data, Count, 0);
	if (BytesSentResult == -1)
	{
		return FALSE;
	}
	if (BytesSent)
	{
		*BytesSent = BytesSentResult;
	}

	return BytesSentResult >= 0;
}

LUMORA_C_API bool8 SendTo(FSocket* Socket, FInternetAddress TargetAddress, const uint8* Data, int32 Count, int32* BytesSent)
{
	LUMORA_ASSERT_MSG(Socket, "Socket is null.");
	LUMORA_ASSERT_MSG(Socket->InternalData, "Socket internal data is null.");

	if (BytesSent)
	{
		*BytesSent = 0;
	}
	
	if (Data == NULL)
	{
		LUMORA_WARN("Socket SendTo failed. Buffer data to send is null.");
		return FALSE;
	}
	if (Count < 0)
	{
		LUMORA_WARN("Socket SendTo failed. Invalid data size.");
		return FALSE;
	}

	const ESocketType SocketType = ((FInternalSocketState*)Socket->InternalData)->SocketType;
	if (SocketType == SOCKET_TYPE_STREAMING)
	{
		LUMORA_INFO("Socket type is stream; do not call SendTo() when using TCP socket.");
		return Send(Socket, Data, Count, BytesSent);
	}

	struct sockaddr_in TargetAddr = { 0 };
	TargetAddr.sin_family = AF_INET;
	TargetAddr.sin_port = htons(TargetAddress.Port);

	int32 Result = inet_pton(AF_INET, TargetAddress.Ip, (void*)&TargetAddr.sin_addr);
	if (Result != 1)
	{
		LUMORA_WARN("Socket SendTo failed; Invalid IP.");
		return FALSE;
	}

	int32 LnxSocket = ((FInternalSocketState*)Socket->InternalData)->Socket;
	int32 BytesSentResult = sendto(LnxSocket, (const char*)Data, Count, 0, (const struct sockaddr*)&TargetAddr, sizeof(struct sockaddr_in));
	if (BytesSentResult == -1)
	{
		return FALSE;
	}
	if (BytesSent)
	{
		*BytesSent = BytesSentResult;
	}

	return BytesSentResult >= 0;
}

LUMORA_C_API bool8 Recv(FSocket* Socket, uint8* Data, int32 Count, int32* BytesRead, ESocketReceiveFlags Flags)
{
	LUMORA_ASSERT_MSG(Socket, "Socket is null.");
	LUMORA_ASSERT_MSG(Socket->InternalData, "Socket internal data is null.");

	if (BytesRead)
	{
		*BytesRead = 0;
	}

	if (Data == NULL)
	{
		LUMORA_WARN("Socket Recv failed. Buffer data to receive is null.");
		return FALSE;
	}
	if (Count < 0)
	{
		LUMORA_WARN("Socket Send failed. Invalid data size.");
		return FALSE;
	}

	const int32 RecvFlag = TranslateRecvFlags(Flags);

	int32 LnxSocket = ((FInternalSocketState*)Socket->InternalData)->Socket;
	int32 BytesReadResult = recv(LnxSocket, (char*)Data, Count, RecvFlag);
	if (BytesReadResult == -1)
	{
		return FALSE;
	}
	if (BytesRead)
	{
		*BytesRead = BytesReadResult;
	}

	return BytesReadResult >= 0;
}

LUMORA_C_API bool8 Close(FSocket* Socket)
{
	LUMORA_ASSERT_MSG(Socket, "Socket is null.");
	LUMORA_ASSERT_MSG(Socket->InternalData, "Socket internal data is null.");

	int32 LnxSocket = ((FInternalSocketState*)Socket->InternalData)->Socket;
	if (LnxSocket != -1)
	{
		int32 Result = close(LnxSocket);
		if (Result == -1)
		{
			return FALSE;
		}

		FInternalSocketState* InternalSocketState = ((FInternalSocketState*)Socket->InternalData);
		InternalSocketState->Socket = -1;
		InternalSocketState->bSocketClosed = TRUE;
		return TRUE;
	}

	return FALSE;
}

LUMORA_C_API bool8 Shutdown(FSocket* Socket, ESocketShutdownMode ShutdownMode)
{
	LUMORA_ASSERT_MSG(Socket, "Socket is null.");
	LUMORA_ASSERT_MSG(Socket->InternalData, "Socket internal data is null.");

	FInternalSocketState* InternalSocketState = ((FInternalSocketState*)Socket->InternalData);

	int32 LnxSocket = InternalSocketState->Socket;
	if (LnxSocket != -1)
	{
		int32 Result = shutdown(LnxSocket, TranslateShutdownMode(ShutdownMode));
		if (Result == -1)
		{
			return FALSE;
		}

		InternalSocketState->bSocketShutdown = TRUE;
		InternalSocketState->ShutdownMode |= ShutdownMode;
		return TRUE;
	}

	return FALSE;
}

LUMORA_C_API bool8 IsValidSocket(const FSocket* Socket)
{
	if (Socket == NULL)
	{
		return FALSE;
	}
	if (Socket->InternalData == NULL)
	{
		return FALSE;
	}
	
	const FInternalSocketState* SocketState = (FInternalSocketState*)Socket->InternalData;
	if (!SocketState->bSocketInitialized)
	{
		return FALSE;
	}
	if (SocketState->Socket == -1)
	{
		return FALSE;
	}

	return TRUE;
}

int32 TranslateRecvFlags(ESocketReceiveFlags Flags)
{
	int32 OutFlags = 0;
	if (Flags & SOCKET_RECEIVE_FLAG_PEEK)
	{
		OutFlags = MSG_PEEK;
	}
	if (Flags & SOCKET_RECEIVE_FLAG_WAITALL)
	{
		OutFlags = MSG_WAITALL;
	}

	return OutFlags;
}

int32 TranslateShutdownMode(ESocketShutdownMode Mode)
{
	int32 OutFlags = 0;
	if (Mode == SOCKET_SHUTDOWN_MODE_READ)
	{
		OutFlags = SHUT_RD;
	}
	if (Mode == SOCKET_SHUTDOWN_MODE_WRITE)
	{
		OutFlags = SHUT_WR;
	}
	if (Mode == SOCKET_SHUTDOWN_MODE_READWRITE)
	{
		OutFlags = SHUT_RDWR;
	}
	return OutFlags;
}

bool8 GetLocalAddressBySocket(const FSocket* Socket, FInternetAddress* OutInternetAddress)
{
	const FInternalSocketState* SocketState = ((FInternalSocketState*)Socket->InternalData);

	struct sockaddr_in LocalAddr = { 0 };
	socklen_t LocalAddrLen = sizeof(LocalAddr);
	int32 NameResult = getsockname(SocketState->Socket, (struct sockaddr*)&LocalAddr, &LocalAddrLen);
	if (NameResult == -1)
	{
		LUMORA_WARN("Cannot store Local address.");
		return FALSE;
	}

	const char* Result = inet_ntop(AF_INET, &LocalAddr.sin_addr, OutInternetAddress->Ip, INET_ADDRSTRLEN);
	if (!Result)
	{
		LUMORA_WARN("Failed to convert local socket address.");
		return FALSE;
	}

	OutInternetAddress->Port = ntohs(LocalAddr.sin_port);
	return TRUE;
}

#endif


