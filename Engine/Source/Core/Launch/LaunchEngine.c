#include "LaunchEngine.h"

#include "Types.h"
#include "Logger.h"
#include "Platform/Platform.h"
#include "HAL/LumoraMemory.h"
#include "Misc/CoreEvent.h"
#include "InputCore/Input.h"

typedef struct FApplicationState
{
    struct FGame* GameInstance;
    bool8 bIsRunning;
    bool8 bIsSuspended;
    FPlatformState PlatformState;
    int16 Width;
    int16 Height;
    float64 LastTime;
} FApplicationState;

static bool8 bInitialized = FALSE;
static FApplicationState GApplicationState;

LUMORA_C_API bool8 ApplicationCreate(struct FGame* GameInstance)
{
    if (bInitialized)
    {
        LUMORA_ERROR("ApplicationCreate called more then once.");
        return FALSE;
    }

    GApplicationState.GameInstance = GameInstance;

    /** Initialize subsystems. */
    InitializeMemory();
    InitializeLogging();
    InitializeEvent();
    InitializeInput();

    GApplicationState.bIsRunning = TRUE;
    GApplicationState.bIsSuspended = FALSE;

    FApplicationConfig* Config = &GApplicationState.GameInstance->ApplicationConfig;
    if (!PlatformStartup(&GApplicationState.PlatformState, Config->ApplicationName, Config->StartPositionX, Config->StartPositionY, Config->StartWidth, Config->StartHeight)) 
    {
        return FALSE;
    }

    if (!GApplicationState.GameInstance->InitializeFunc(GApplicationState.GameInstance))
    {
        LUMORA_FATAL("Game failed to initialize.");
        return FALSE;
    }

    GApplicationState.GameInstance->OnResizeFunc(GApplicationState.GameInstance, GApplicationState.Width, GApplicationState.Height);
    bInitialized = TRUE;

    return TRUE;
}

LUMORA_C_API bool8 ApplocationLoop()
{
    LUMORA_INFO(HGetMemoryUseageStr());

    while (GApplicationState.bIsRunning)
    {
        if (!PlatformPumpMessage(&GApplicationState.PlatformState))
        {
            GApplicationState.bIsRunning = FALSE;
        }

        if (!GApplicationState.bIsSuspended)
        {
            if (!GApplicationState.GameInstance->UpdateFunc(GApplicationState.GameInstance, (float32)0))
            {
                LUMORA_FATAL("Game update failed, shutting down.");
                GApplicationState.bIsRunning = FALSE;
                break;
            }

            if (!GApplicationState.GameInstance->RenderFunc(GApplicationState.GameInstance, (float32)0))
            {
                LUMORA_FATAL("Game render failed, shutting down.");
                GApplicationState.bIsRunning = FALSE;
                break;
            }

            /**
             * NOTE: Input update/state copying should always be handled
             * after any input should be recorded; E.E. before this line.
             * As a safety, input is the last thing to be updated before
             * this frame ends.
             */
            UpdateInput(0);
        }
    }

    GApplicationState.bIsRunning = FALSE;
    
    PlatformShutdown(&GApplicationState.PlatformState);
    ShutdownLogging();
    ReleaseEvent();
    ReleaseInput();
    ReleaseMemory();

    return TRUE;
}
