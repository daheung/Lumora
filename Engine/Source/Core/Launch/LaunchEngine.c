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

/** Event handlers */
static bool8 ApplicationOnEvent(uint16 Code, void* Sender, void* ListenerList, FCoreEventContext Context);
static bool8 ApplicationOnKey(uint16 Code, void* Sender, void* ListenerList, FCoreEventContext Context);

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

    /** Register event for quit engine. */
    RegisterEvent(EVENT_CODE_APPLICATION_QUIT, 0, ApplicationOnEvent);
    RegisterEvent(EVENT_CODE_KEY_PRESSED     , 0, ApplicationOnKey);
    RegisterEvent(EVENT_CODE_KEY_RELEASED    , 0, ApplicationOnKey);

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
    
    /** Unregister events. */
    UnregisterEvent(EVENT_CODE_APPLICATION_QUIT, 0, ApplicationOnEvent);
    UnregisterEvent(EVENT_CODE_KEY_PRESSED     , 0, ApplicationOnKey);
    UnregisterEvent(EVENT_CODE_KEY_RELEASED    , 0, ApplicationOnKey);

    PlatformShutdown(&GApplicationState.PlatformState);
    ShutdownLogging();
    ReleaseEvent();
    ReleaseInput();
    ReleaseMemory();

    return TRUE;
}

bool8 ApplicationOnEvent(uint16 Code, void *Sender, void *ListenerList, FCoreEventContext Context)
{
    switch (Code)
    {
    case EVENT_CODE_APPLICATION_QUIT:
        LUMORA_INFO("EVENT_CODE_APPLICATION_QUIT received, shutting down.");
        GApplicationState.bIsRunning = FALSE;
        return TRUE;
    }

    return FALSE;
}

bool8 ApplicationOnKey(uint16 Code, void *Sender, void *ListenerList, FCoreEventContext Context)
{
    switch (Code)
    {
    case EVENT_CODE_KEY_PRESSED:
    {
        uint16 KeyCode = Context.Data.U16[0];
        if (KeyCode == KEY_ESCAPE)
        {
            /** NOTE: Technically firing an event to itself, but there may be other listeners. */
            FCoreEventContext EventContext = {};
            FireEvent(EVENT_CODE_APPLICATION_QUIT, 0, EventContext);

            /** Block anything else from processing this. */
            return TRUE;
        }
        else if (KeyCode == KEY_A)
        {
            /** Example on checking for a key */
            LUMORA_DEBUG("Explicit - A key pressed.");
        } 
        else 
        {
            LUMORA_DEBUG("'%c' key pressed in window.", KeyCode);
        }
    }
        break;
    case EVENT_CODE_KEY_RELEASED:
    {
        uint16 KeyCode = Context.Data.U16[0];
        if (KeyCode == KEY_B)
        {
            /** Example on checking for a key */
            LUMORA_DEBUG("Explicit - B key pressed.");
        }
        else
        {
            LUMORA_DEBUG("'%c' key released in window.", KeyCode);
        }
    }
        break;
    }

    return FALSE;
}
