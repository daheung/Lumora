#include "Game.h"

#include <LaunchMain.h>

/** TODO: Remove this */
#include <HAL/LumoraMemory.h>

/** Define the function to create a game. */
bool8 CreateGame(FGame* OutGame)
{
    /** Application configuration. */
    OutGame->ApplicationConfig.StartPositionX = 100;
    OutGame->ApplicationConfig.StartPositionY = 100;
    OutGame->ApplicationConfig.StartWidth = 1280;
    OutGame->ApplicationConfig.StartHeight = 720;
    OutGame->ApplicationConfig.ApplicationName = "Lumora Game Engine";

    OutGame->InitializeFunc = GameInitialize;
    OutGame->UpdateFunc = GameUpdate;
    OutGame->RenderFunc = GameRender;
    OutGame->OnResizeFunc = GameOnResize;

    /** Create the game state. */
    OutGame->State = HAllocate(sizeof(FGameState), MEMORY_TAG_GAME);

    return TRUE;
}