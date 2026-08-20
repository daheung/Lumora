#pragma once

#include <Defines.h>
#include <Types.h>

typedef struct 
{
    float32 DeltaTime;
} FGameState;

bool8 GameInitialize(struct FGame* GameInstance);
bool8 GameUpdate(struct FGame* GameInstance, float32 DeltaTime);
bool8 GameRender(struct FGame* GameInstance, float32 DeltaTime);
void GameOnResize(struct FGame* GameInstance, uint32 Width, uint32 Height);
bool8 GameRelease(struct FGame* GameInstance);