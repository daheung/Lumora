#include "EntryPoint.h"

int main(void) {
    // InitializeMemory();
    // InitializeLogging();

    /** Request the game instance from the application. */
    FGame GameInstance = { 0 };
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

    // ReleaseMemory();
    // ShutdownLogging();
    return 0;
}