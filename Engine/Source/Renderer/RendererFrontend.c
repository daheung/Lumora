#include "RendererFrontend.h"
#include "RendererBackend.h"

#include "Core/Logger.h"
#include "Core/HAL/LumoraMemory.h"

/** Backend render context. */
//static FRendererBackend* GBackend = NULL;

typedef struct FRendererSystemState
{
    FRendererBackend Backend;
} FRendererSystemState;

static FRendererSystemState* GRendererState;

bool8 InitializeRenderer(size_t* MemoryRequirement, void* State, const char* ApplicationName)
{
    *MemoryRequirement = sizeof(FRendererSystemState);
    if (State == 0) {
        return TRUE;
    }

    GRendererState = State;
    HZeroMemory(GRendererState, sizeof(FRendererSystemState));

    /** TODO: Mack this configurable. */
    CreateRendererBackend(RENDERER_BACKEND_TYPE_VULKAN, &GRendererState->Backend);
    GRendererState->Backend.FrameCount = 0;

    const bool8 bInitSucceed = GRendererState->Backend.Initialize(&GRendererState->Backend, ApplicationName);
    if (!bInitSucceed)
    {
        LUMORA_FATAL("Renderer backend failed to initialize. Shutting down.");
        return FALSE;
    }

    return TRUE;
}

void ReleaseRenderer(void* State)
{
    LUMORA_UNUSED_PARAM(State);

    if (GRendererState) 
    {
        GRendererState->Backend.Release(&GRendererState->Backend);
    }
    GRendererState = NULL;
}

void RendererOnResize(uint16 Width, uint16 Height)
{
    if (GRendererState && GRendererState->Backend.Resized)
    {
        GRendererState->Backend.Resized(&GRendererState->Backend, Width, Height);
    }
    else
    {
        LUMORA_WARN("Renderer backend does not exist to accept resize: (%i, %i)", Width, Height);
    }
}

bool8 RendererBeginFrame(float32 DeltaTime)
{
    if (GRendererState == NULL)
    {
        return FALSE;
    }

    return GRendererState->Backend.BeginFrame(&GRendererState->Backend, DeltaTime);
}

bool8 RendererEndFrame(float32 DeltaTime)
{
    const bool8 bEndFrameSucceed = GRendererState->Backend.EndFrame(&GRendererState->Backend, DeltaTime);
    GRendererState->Backend.FrameCount++;
    return bEndFrameSucceed;
}

bool8 RendererDrawFrame(FRenderPacket *Packet)
{
    /** If the begin returned successfully, mid-frame operations may continue. */
    if (RendererBeginFrame(Packet->DeltaTime))
    {
        /** End the frame. If this fails, it is likely unrecoverable. */
        bool8 bEndFrameSucceed = RendererEndFrame(Packet->DeltaTime);
        if (!bEndFrameSucceed)
        {
            LUMORA_ERROR("RendererEndFrame failed. Application Shutting down.");
            return FALSE;
        }
    }

    return TRUE;
}
