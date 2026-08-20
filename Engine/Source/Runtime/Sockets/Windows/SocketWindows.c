#include "SocketWindows.h"

#if PLATFORM_WINDOWS

#include "Runtime/Sockets/SocketTypes.h"

#include "Core/HAL/LumoraMemory.h"
#include "Core/Misc/CString.h"
#include "Core/Logger.h"

#include <winsock2.h>
#include <Windows.h>
#include <WS2tcpip.h>

typedef struct FInternalSocketState
{
	SOCKET Socket;

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
	SOCKET WinSocket = INVALID_SOCKET;
	switch (SocketType)
	{
	case SOCKET_TYPE_DATAGRAM:
		WinSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		break;
	case SOCKET_TYPE_STREAMING:
		WinSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		break;
	default:
		LUMORA_ERROR("Failed to create socket.");
		return (FSocket) { NULL };
	}
	
	if (WinSocket == INVALID_SOCKET)
	{
		LUMORA_ERROR("Failed to create socket.");
		return (FSocket){ NULL };
	}

	FInternalSocketState* SocketState = HAllocate(sizeof(FInternalSocketState), MEMORY_TAG_APPLICATION);
	HZeroMemory(SocketState, sizeof(FInternalSocketState));
	SocketState->Socket = WinSocket;
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
	if (BindAddress.Ip)
	{
		int32 Result = inet_pton(AF_INET, BindAddress.Ip, (void*)&Ip);
		if (Result != 1)
		{
			LUMORA_WARN("Bind WinSock failed; Invalid Ip.");
			return FALSE;
		}
	}
	else
	{
		Ip = htonl(INADDR_ANY);
	}

	FInternalSocketState* SocketState = ((FInternalSocketState*)Socket->InternalData);

	SOCKADDR_IN SvrAddr = { 0 };
	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_port = htons(BindAddress.Port);
	SvrAddr.sin_addr.S_un.S_addr = Ip;

	SOCKET WinSocket = SocketState->Socket;
	int32 Result = bind(WinSocket, (const SOCKADDR*)&SvrAddr, sizeof(SOCKADDR_IN));
	if (Result != SOCKET_ERROR)
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

	if (!DestAddress.Ip)
	{
		LUMORA_WARN("Destination IP is null.");
		return FALSE;
	}

	uint32 Ip = 0;
	int32 Result = inet_pton(AF_INET, DestAddress.Ip, (void*)&Ip);
	if (Result != 1)
	{
		LUMORA_WARN("Connect WinSock failed; Invalid Ip.");
		return FALSE;
	}

	FInternalSocketState* ClientSocketState = ((FInternalSocketState*)Socket->InternalData);

	SOCKADDR_IN SvrAddr = { 0 };
	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_port = htons(DestAddress.Port);
	SvrAddr.sin_addr.S_un.S_addr = Ip;

	SOCKET WinSocket = ClientSocketState->Socket;
	const int32 ConnectResult = connect(WinSocket, (const SOCKADDR*)&SvrAddr, sizeof(SOCKADDR_IN));
	if (ConnectResult != SOCKET_ERROR)
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
	SOCKET WinSocket = ((FInternalSocketState*)Socket->InternalData)->Socket;
	return listen(WinSocket, MaxBacklog) != SOCKET_ERROR;
}

LUMORA_C_API FSocket Accept(FSocket* Socket)
{
	LUMORA_ASSERT_MSG(Socket, "Socket is null.");
	LUMORA_ASSERT_MSG(Socket->InternalData, "Socket internal data is null.");

	const FInternalSocketState* ServerSocketState = ((FInternalSocketState*)Socket->InternalData);
	
	SOCKADDR_IN ClientAddr = { 0 };
	int32 ClientAddrLen = sizeof(ClientAddr);
	SOCKET ClientWinSocket = accept(ServerSocketState->Socket, (SOCKADDR*)&ClientAddr, &ClientAddrLen);
	if (ClientWinSocket != INVALID_SOCKET)
	{
		FSocket OutSocket = { 0 };
		FInternalSocketState* ClientSocketState = HAllocate(sizeof(FInternalSocketState), MEMORY_TAG_APPLICATION);
		HZeroMemory(ClientSocketState, sizeof(FInternalSocketState));
		ClientSocketState->Socket			= ClientWinSocket;
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
		LUMORA_WARN("WinSocket Send failed. Buffer data to send is null.");
		return FALSE;
	}
	if (Count < 0)
	{
		LUMORA_WARN("WinSocket Send failed. Invalid data size.");
		return FALSE;
	}

	SOCKET WinSocket = ((FInternalSocketState*)Socket->InternalData)->Socket;
	int32 BytesSentResult = send(WinSocket, (const char*)Data, Count, 0);
	if (BytesSentResult == SOCKET_ERROR)
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
		LUMORA_WARN("WinSocket SendTo failed. Buffer data to send is null.");
		return FALSE;
	}
	if (Count < 0)
	{
		LUMORA_WARN("WinSocket SendTo failed. Invalid data size.");
		return FALSE;
	}

	const ESocketType SocketType = ((FInternalSocketState*)Socket->InternalData)->SocketType;
	if (SocketType == SOCKET_TYPE_STREAMING)
	{
		LUMORA_INFO("Socket type is stream; do not call SendTo() when using TCP socket.");
		return Send(Socket, Data, Count, BytesSent);
	}

	if (!TargetAddress.Ip)
	{
		LUMORA_WARN("WinSocket SendTo failed; destination IP is null.");
		return FALSE;
	}

	SOCKADDR_IN TargetAddr = { 0 };
	TargetAddr.sin_family = AF_INET;
	TargetAddr.sin_port = htons(TargetAddress.Port);

	int32 Result = inet_pton(AF_INET, TargetAddress.Ip, (void*)&TargetAddr.sin_addr.S_un);
	if (Result != 1)
	{
		LUMORA_WARN("WinSocket SendTo failed; Invalid IP.");
		return FALSE;
	}

	SOCKET WinSocket = ((FInternalSocketState*)Socket->InternalData)->Socket;
	int32 BytesSentResult = sendto(WinSocket, (const char*)Data, Count, 0, (const SOCKADDR*)&TargetAddr, sizeof(SOCKADDR_IN));
	if (BytesSentResult == SOCKET_ERROR)
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
		LUMORA_WARN("WinSocket Recv failed. Buffer data to receive is null.");
		return FALSE;
	}
	if (Count < 0)
	{
		LUMORA_WARN("WinSocket Send failed. Invalid data size.");
		return FALSE;
	}

	const int32 RecvFlag = TranslateRecvFlags(Flags);

	SOCKET WinSocket = ((FInternalSocketState*)Socket->InternalData)->Socket;
	int32 BytesReadResult = recv(WinSocket, (char*)Data, Count, RecvFlag);
	if (BytesReadResult == SOCKET_ERROR)
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

	SOCKET WinSocket = ((FInternalSocketState*)Socket->InternalData)->Socket;
	if (WinSocket != INVALID_SOCKET)
	{
		int32 Result = closesocket(WinSocket);
		if (Result == SOCKET_ERROR)
		{
			return FALSE;
		}

		FInternalSocketState* InternalSocketState = ((FInternalSocketState*)Socket->InternalData);
		InternalSocketState->Socket = INVALID_SOCKET;
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

	SOCKET WinSocket = InternalSocketState->Socket;
	if (WinSocket != INVALID_SOCKET)
	{
		int32 Result = shutdown(WinSocket, TranslateShutdownMode(ShutdownMode));
		if (Result == SOCKET_ERROR)
		{
			return FALSE;
		}

		InternalSocketState->bSocketShutdown = TRUE;
		InternalSocketState->ShutdownMode |= ShutdownMode;
		return TRUE;
	}

	return FALSE;
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
		OutFlags = SD_RECEIVE;
	}
	if (Mode == SOCKET_SHUTDOWN_MODE_WRITE)
	{
		OutFlags = SD_SEND;
	}
	if (Mode == SOCKET_SHUTDOWN_MODE_READWRITE)
	{
		OutFlags = SD_BOTH;
	}
	return OutFlags;
}

bool8 GetLocalAddressBySocket(const FSocket* Socket, FInternetAddress* OutInternetAddress)
{
	const FInternalSocketState* SocketState = ((FInternalSocketState*)Socket->InternalData);

	SOCKADDR_IN LocalAddr = { 0 };
	int32 LocalAddrLen = sizeof(LocalAddr);
	int32 NameResult = getsockname(SocketState->Socket, (SOCKADDR*)&LocalAddr, &LocalAddrLen);
	if (NameResult == SOCKET_ERROR)
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


