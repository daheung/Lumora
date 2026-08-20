#pragma once

#include "Defines.h"

#define IPV4_ADDRESS_STRING_LENGTH 22

typedef enum ESocketSubsystemType
{
	SOCKET_SUBSYSTEM_TYPE_WINDOWS,
	SOCKET_SUBSYSTEM_TYPE_LINUX,
} ESocketSubsystemType;

typedef enum ESocketType
{
	/** Not bound. */
	SOCKET_TYPE_UNKNOWN,

	/** UDP */
	SOCKET_TYPE_DATAGRAM,

	/** TCP */
	SOCKET_TYPE_STREAMING,
} ESocketType;

typedef enum ESocketReceiveFlags
{
	/** Default Mode */
	SOCKET_RECEIVE_FLAG_NONE,

	SOCKET_RECEIVE_FLAG_PEEK,
	
	SOCKET_RECEIVE_FLAG_WAITALL,
} ESocketReceiveFlags;

typedef enum ESocketShutdownMode
{
	SOCKET_SHUTDOWN_MODE_KNOWN		= 0,
	SOCKET_SHUTDOWN_MODE_READ		= 1 << 0,
	SOCKET_SHUTDOWN_MODE_WRITE		= 1 << 1,
	SOCKET_SHUTDOWN_MODE_READWRITE	= SOCKET_SHUTDOWN_MODE_READ | SOCKET_SHUTDOWN_MODE_WRITE
} ESocketShutdownMode;

typedef struct FInternetAddress
{
	char Ip[IPV4_ADDRESS_STRING_LENGTH];
	uint16 Port;
} FInternetAddress;