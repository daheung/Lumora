#pragma once

#include "Defines.h"

struct FPlatformState;
struct FVulkanContext;

bool8 PlatformCreateVulkanSurface(struct FVulkanContext* VulkanContext);

/**
 * Appends the names of required extensions for this platform to
 * the CArrayNames, which should be created and passed in.
 */
void PlatformGetRequiredExtensionNames(const char*** CArrayNames);