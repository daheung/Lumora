#pragma once

#include "Renderer/RendererBackend.h"

struct FPlatformState;

bool8 VulkanInitializeRendererBackend(FRendererBackend* Backend, const char* ApplicationName, struct FPlatformState* PlatformState);

void VulkanReleaseRendererBackend(FRendererBackend* Backend);

void VulkanRendererOnResized(FRendererBackend* Backend, uint16 Width, uint16 Height);

bool8 VulkanRendererBackendBeginFrame(FRendererBackend* Backend, float32 DeltaTime);

bool8 VulkanRendererBackendEndFrame(FRendererBackend* Backend, float32 DeltaTime);