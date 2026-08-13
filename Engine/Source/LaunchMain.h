#pragma once

#include "Core/Launch/LaunchEngine.h"
#include "Core/Logger.h"
#include "HAL/LumoraMemory.h"
#include "Types.h"

/** Externally-defined function to create a game. */
extern bool8 CreateGame(FGame* OutGame);

int main(void) {
    InitializeMemory();
    InitializeLogging();

    /** Request the game instance from the application. */
    FGame GameInstance;
    if (!CreateGame(&GameInstance))
    {
        LUMORA_FATAL("Could not create game.");
        return -1;
    }

    if (!GameInstance.RenderFunc || !GameInstance.UpdateFunc || !GameInstance.InitializeFunc || !GameInstance.OnResizeFunc)
    {
        LUMORA_FATAL("The game's function pointers must be assigned.");
        return -2;
    }

    /** Initialization. */
    if (!ApplicationCreate(&GameInstance))
    {
        LUMORA_INFO("Application failed to create.");
        return 1;
    }

    /** Begin the game loop. */
    if (!ApplocationLoop())
    {
        LUMORA_INFO("Application did not shutdown gracefully.");
        return 2;
    }

    ReleaseMemory();
    ShutdownLogging();
    return 0;
}