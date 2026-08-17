#pragma once

#include "RendererTypes.inl"

struct FStaticMeshData;
struct FPlatformState;
struct FRenderPacket;

bool8 InitializeRenderer(const char* ApplicationName, struct FPlatformState* PlatformState);

void ReleaseRenderer();

void RendererOnResize(uint16 Width, uint16 Height);

bool8 RendererDrawFrame(FRenderPacket* Packet);