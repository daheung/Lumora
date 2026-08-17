#include "RendererFrontend.h"
#include "RendererBackend.h"

#include "Core/Logger.h"
#include "Core/HAL/LumoraMemory.h"

/** Backend render context. */
static FRendererBackend* GBackend = NULL;

bool8 InitializeRenderer(const char* ApplicationName, struct FPlatformState* PlatformState)
{
    GBackend = HAllocate(sizeof(FRendererBackend), MEMORY_TAG_RENDERER);

    /** TODO: Mack this configurable. */
    CreateRendererBackend(RENDERER_BACKEND_TYPE_VULKAN, PlatformState, GBackend);
    GBackend->FrameCount = 0;

    const bool8 bInitSucceed = GBackend->Initialize(GBackend, ApplicationName, PlatformState);
    if (!bInitSucceed)
    {
        LUMORA_FATAL("Renderer backend failed to initialize. Shutting down.");
        return FALSE;
    }

    return TRUE;
}

void ReleaseRenderer()
{
    GBackend->Release(GBackend);
    HFree(GBackend, sizeof(FRendererBackend), MEMORY_TAG_RENDERER);
}

void RendererOnResize(uint16 Width, uint16 Height)
{
}

bool8 RendererBeginFrame(float32 DeltaTime)
{
    return GBackend->BeginFrame(GBackend, DeltaTime);
}

bool8 RendererEndFrame(float32 DeltaTime)
{
    const bool8 bEndFrameSucceed = GBackend->EndFrame(GBackend, DeltaTime);
    GBackend->FrameCount++;
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
