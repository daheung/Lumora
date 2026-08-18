#include "LaunchEngine.h"

#include "Types.h"
#include "Logger.h"
#include "Platform/Platform.h"
#include "HAL/LumoraMemory.h"
#include "Misc/CoreEvent.h"
#include "Misc/Clock.h"
#include "InputCore/Input.h"

#include "Renderer/RendererFrontend.h"
typedef struct FApplicationState
{
    struct FGame* GameInstance;
    bool8 bIsRunning;
    bool8 bIsSuspended;
    FPlatformState PlatformState;
    int16 Width;
    int16 Height;
    FClock Clock;
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

    /** Initialize Renderer */
    const bool8 bRendererInitSucceed = InitializeRenderer(GameInstance->ApplicationConfig.ApplicationName, &GApplicationState.PlatformState);
    if (!bRendererInitSucceed)
    {
        LUMORA_FATAL("Failed to initialize renderer. Aborting applocation.");
        return FALSE;
    }

    /** This is really a core count. Subtract 1 to account for the main thread already being in use. */
    int32 ThreadCount = PlatformGetProcessorCount() - 1;
    if (ThreadCount < 1)
    {
        LUMORA_FATAL("Error: Platform reported processor count (minus one for main thread) as %i. Need at last one additional thread for the job system.", ThreadCount);
        return FALSE;
    }

    LUMORA_TRACE("Available threads: %i", ThreadCount);

    /** Cap the thread count. */
    const int32 MaxThreadCount = 16;
    if (ThreadCount > MaxThreadCount)
    {
        LUMORA_TRACE("Available threads on the system is %i, but will be capped at %i.", ThreadCount, MaxThreadCount);
        ThreadCount = MaxThreadCount;
    }
    
    /** Initialize the Game */
    const bool8 bGameInitSucceed = GApplicationState.GameInstance->InitializeFunc(GApplicationState.GameInstance);
    if (!bGameInitSucceed)
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

    StartClock(&GApplicationState.Clock);
    UpdateClock(&GApplicationState.Clock);
    GApplicationState.LastTime = GApplicationState.Clock.ElapsedTime;

    float64 RunningTime = 0;
    uint8 FrameCount = 0;
    float64 TargetFrameSeconds = 1.0f / 60;

    while (GApplicationState.bIsRunning)
    {
        if (!PlatformPumpMessage(&GApplicationState.PlatformState))
        {
            GApplicationState.bIsRunning = FALSE;
        }

        if (!GApplicationState.bIsSuspended)
        {
            /** Update clock and get delta time. */
            UpdateClock(&GApplicationState.Clock);
            float64 CurTime = GApplicationState.Clock.ElapsedTime;
            float64 DeltaTime = (CurTime - GApplicationState.LastTime);
            float64 FrameStartTime = PlatformGetAbsoluteTime();
            
            const bool8 bUpdateSucceed = GApplicationState.GameInstance->UpdateFunc(GApplicationState.GameInstance, (float32)DeltaTime);
            if (!bUpdateSucceed)
            {
                LUMORA_FATAL("Game update failed, shutting down.");
                GApplicationState.bIsRunning = FALSE;
                break;
            }

            const bool8 bRenderSucceed = GApplicationState.GameInstance->RenderFunc(GApplicationState.GameInstance, (float32)DeltaTime);
            if (!bRenderSucceed)
            {
                LUMORA_FATAL("Game render failed, shutting down.");
                GApplicationState.bIsRunning = FALSE;
                break;
            }

            /** TODO: Refactor packet creation */
            FRenderPacket Packet = {};
            Packet.DeltaTime = DeltaTime;
            RendererDrawFrame(&Packet);

            /** Figure out how long the frame took and, if below. */
            float64 FrameEndTime = PlatformGetAbsoluteTime();
            float64 FrameElapsedTime = FrameEndTime - FrameStartTime;
            RunningTime += FrameElapsedTime;
            float64 RemainingSeconds = TargetFrameSeconds - FrameElapsedTime;
            
            if (RemainingSeconds > 0)
            {
                uint64 RemainingMs = (RemainingSeconds * 1000);

                /** If there is time left, give it back to the OS. */
                bool8 LimitFrames = FALSE;
                if (RemainingMs > 0 && LimitFrames)
                {
                    PlatformSleep(RemainingMs - 1);
                }

                FrameCount++;
            }

            /**
             * NOTE: Input update/state copying should always be handled
             * after any input should be recorded; E.E. before this line.
             * As a safety, input is the last thing to be updated before
             * this frame ends.
             */
            UpdateInput(0);

            /** Update last time */
            GApplicationState.LastTime = CurTime;
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
    ReleaseRenderer();
    
    return TRUE;
}

bool8 ApplicationOnEvent(uint16 Code, void* Sender, void* ListenerList, FCoreEventContext Context)
{
    LUMORA_UNUSED_PARAM(Sender);
    LUMORA_UNUSED_PARAM(ListenerList);
    LUMORA_UNUSED_PARAM(Context);

    switch (Code)
    {
    case EVENT_CODE_APPLICATION_QUIT:
        LUMORA_INFO("EVENT_CODE_APPLICATION_QUIT received, shutting down.");
        GApplicationState.bIsRunning = FALSE;
        return TRUE;
    }

    return FALSE;
}

bool8 ApplicationOnKey(uint16 Code, void* Sender, void* ListenerList, FCoreEventContext Context)
{
    LUMORA_UNUSED_PARAM(Sender);
    LUMORA_UNUSED_PARAM(ListenerList);
    
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
