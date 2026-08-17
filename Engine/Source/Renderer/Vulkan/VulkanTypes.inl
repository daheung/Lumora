#pragma once

#include "Defines.h"
#include "Asserts.h"

#include <vulkan/vulkan.h>

/** Checks the given expression's return value against VK_SUCCESS. */
#define VK_CHECK(Expr) LUMORA_ASSERT(Expr == VK_SUCCESS)

typedef struct FVulkanSwapchainSupportInfo
{
    VkSurfaceCapabilitiesKHR Capabilities;
    uint32 FormatCount;
    VkSurfaceFormatKHR* Formats;
    uint32 PresentModeCount;
    VkPresentModeKHR* PresentModes;
} FVulkanSwapchainSupportInfo;

typedef struct FVulkanDevice
{
    VkPhysicalDevice PhysicalDevice;
    VkDevice Device;
    FVulkanSwapchainSupportInfo SwapchainSupport;

    int32 GraphicsQueueIndex;
    int32 PresentQueueIndex;
    int32 TransferQueueIndex;

    VkQueue GraphicsQueue;
    VkQueue PresentQueue;
    VkQueue TransferQueue;

    VkPhysicalDeviceProperties Properties;
    VkPhysicalDeviceFeatures Features;
    VkPhysicalDeviceMemoryProperties Memory;
} FVulkanDevice;

typedef struct FVulkanContext
{
    VkInstance Instance;
    VkAllocationCallbacks* Allocator;
    VkSurfaceKHR Surface;
    VkDebugUtilsMessengerEXT DebugMessenger;

    FVulkanDevice Device;
} FVulkanContext;