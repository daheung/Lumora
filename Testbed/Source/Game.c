#include "Game.h"

#include <Core/Logger.h>

#include "Runtime/Sockets/Sockets.h"
#include "Runtime/Sockets/SocketSubsystem.h"
#include "Core/Containers/Array.h"

static FSocketSubsystemState GSocketSubsystem;
static FSocket  GClientSocket;

bool8 GameInitialize(struct FGame* GameInstance)
{
    InitializeSocketSubsystem(&GSocketSubsystem);

    GClientSocket = CreateSocket(SOCKET_TYPE_STREAMING);

    FInternetAddress BindAddress = {
        .Ip = "192.168.3.132",
        .Port = 10000
    };
    const bool8 bConnectSucceed = Connect(&GClientSocket, BindAddress);

    LUMORA_DEBUG("GameInitialize() called");
    return TRUE;
}

bool8 GameUpdate(struct FGame *GameInstance, float32 DeltaTime)
{
    return TRUE;
}

bool8 GameRender(struct FGame *GameInstance, float32 DeltaTime)
{
    return TRUE;
}

void GameOnResize(struct FGame *GameInstance, uint32 Width, uint32 Height)
{
}

bool8 GameRelease(struct FGame* GameInstance)
{
    return TRUE;
}