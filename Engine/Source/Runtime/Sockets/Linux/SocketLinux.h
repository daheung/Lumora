#pragma once

#include "Defines.h"

#if defined(PLATFORM_LINUX)

#include "Runtime/Sockets/Sockets.h"

FSocket CreateSocketImpl(ESocketType SocketType);

void ReleaseSocketImpl(FSocket* Socket);

#endif