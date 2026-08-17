#pragma once

#include "RendererTypes.inl"

struct FPlatformState;

bool8 CreateRendererBackend(ERendererBackendType Type, struct FPlatformState* PlatformState, FRendererBackend* OutRendererBackend);

void ReleaseRendererBackend(FRendererBackend* RendererBackend);