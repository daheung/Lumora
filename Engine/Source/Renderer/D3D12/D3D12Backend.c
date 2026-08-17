#include "D3D12Backend.h"

bool8 D3D12InitializeRendererBackend(FRendererBackend* Backend, const char* ApplicationName, struct FPlatformState* PlatformState)
{
    return TRUE;
}

void D3D12ReleaseRendererBackend(FRendererBackend* Backend)
{
}

void D3D12RendererOnResized(FRendererBackend* Backend, uint16 Width, uint16 Height)
{
}

bool8 D3D12RendererBackendBeginFrame(FRendererBackend* Backend, float32 DeltaTime)
{
    return TRUE;
}

bool8 D3D12RendererBackendEndFrame(FRendererBackend* Backend, float32 DeltaTime)
{
    return TRUE;
}
