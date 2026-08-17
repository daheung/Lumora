#include "RendererBackend.h"

#include "Vulkan/VulkanBackend.h"

bool8 CreateRendererBackend(ERendererBackendType Type, struct FPlatformState* PlatformState, FRendererBackend* OutRendererBackend)
{
    OutRendererBackend->PlatformState = PlatformState;

    switch (Type)
    {
    case RENDERER_BACKEND_TYPE_VULKAN:
        OutRendererBackend->Initialize = VulkanInitializeRendererBackend;
        OutRendererBackend->Release = VulkanReleaseRendererBackend;
        OutRendererBackend->BeginFrame = VulkanRendererBackendBeginFrame;
        OutRendererBackend->EndFrame = VulkanRendererBackendEndFrame;
        OutRendererBackend->Resized = VulkanRendererOnResized;
        return TRUE;

    default:
        return FALSE;
    }

    return FALSE;
}

void ReleaseRendererBackend(FRendererBackend* RendererBackend)
{
    RendererBackend->Initialize = NULL;
    RendererBackend->Release = NULL;
    RendererBackend->BeginFrame = NULL;
    RendererBackend->EndFrame = NULL;
    RendererBackend->Resized = NULL;
}
