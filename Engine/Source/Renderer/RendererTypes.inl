#pragma once

#include "Defines.h"

struct FRendererBackend;
struct FPlatformState;

typedef enum ERendererBackendType
{
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECT11,
    RENDERER_BACKEND_TYPE_DIRECT12,
} ERendererBackendType;

typedef struct FRendererBackend
{
    struct FPlatformState* PlatformState;
    uint64 FrameCount;

    bool8 (*Initialize)(struct FRendererBackend* Backend, const char* ApplicationName, struct FPlatformState* PlatformState);

    void  (*Release)(struct FRendererBackend* Backend);

    void  (*Resized)(struct FRendererBackend* Backend, uint16 Width, uint16 Height);

    bool8 (*BeginFrame)(struct FRendererBackend* Backend, float32 DeltaTime);

    bool8 (*EndFrame)(struct FRendererBackend* Backend, float32 DeltaTime);
} FRendererBackend;

typedef struct FRenderPacket
{
    float32 DeltaTime;
} FRenderPacket;