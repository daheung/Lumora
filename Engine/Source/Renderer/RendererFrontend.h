#pragma once

#include "RendererTypes.inl"

struct FStaticMeshData;
struct FPlatformState;
struct FRenderPacket;

bool8 InitializeRenderer(size_t* MemoryRequirement, void* State, const char* ApplicationName);

void ReleaseRenderer(void* State);

void RendererOnResize(uint16 Width, uint16 Height);

bool8 RendererDrawFrame(FRenderPacket* Packet);