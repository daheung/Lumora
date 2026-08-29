#pragma once

#include "RendererTypes.inl"

struct FPlatformState;

bool8 CreateRendererBackend(ERendererBackendType Type, FRendererBackend* OutRendererBackend);

void ReleaseRendererBackend(FRendererBackend* RendererBackend);