#pragma once

#include "Core/Launch/LaunchEngine.h"

/**
 * Represents the basic game state in a game.
 * Called for creation by the application.
 */
typedef struct FGame
{
    /** The application configuration. */
    FApplicationConfig ApplicationConfig;

    /** Function pointer to game's initialize function. */
    bool8 (*InitializeFunc)(struct FGame* GameInstance);

    /** Function pointer to game's update function. */
    bool8 (*UpdateFunc)(struct FGame* GameInstance, float32 DeltaTime);

    /** Function pointer to game's render function. */
    bool8 (*RenderFunc)(struct FGame* GameInstance, float32 DeltaTime);

    /** Function pointer to handle resizes, if applicable. */
    void (*OnResizeFunc)(struct FGame* GameInstance, uint32 Width, uint32 Height);

    /** Game-specific game state. Created and managed by the game. */
    void* State;
} FGame;