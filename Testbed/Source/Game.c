#include "Game.h"

#include <Core/Logger.h>
#include <Core/HAL/LumoraMemory.h>
#include <InputCore/Input.h>

static size_t GAllocationCount = 0;

bool8 GameInitialize(struct FGame* GameInstance)
{
    LUMORA_DEBUG("GameInitialize() called");
    return TRUE;
}

bool8 GameUpdate(struct FGame *GameInstance, float32 DeltaTime)
{
    size_t PrevAllocationCount = GAllocationCount;
    GAllocationCount = GetMemoryAllocationCount();

    if (IsInputKeyUp(KEY_M) && WasInputKeyDown(KEY_M))
    {
        LUMORA_DEBUG("Allocations: %zu (%zu this frame)", GAllocationCount, GAllocationCount - PrevAllocationCount);
    }

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