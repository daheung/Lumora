#include "Game.h"

#include <Core/Logger.h>

bool8 GameInitialize(struct FGame* GameInstance)
{
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