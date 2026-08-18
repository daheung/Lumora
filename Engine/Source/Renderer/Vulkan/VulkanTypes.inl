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

    VkFormat DepthFormat;
} FVulkanDevice;

typedef struct FVulkanImage
{
    VkImage Handle;
    VkDeviceMemory Memory;
    VkImageView ImageView;
    uint32 Width;
    uint32 Height;
} FVulkanImage;

typedef struct FVulkanSwapchain
{
    VkSurfaceFormatKHR ImageFormat;
    uint8 MaxFramesInFlight;
    VkSwapchainKHR Handle;
    uint32 ImageCount;
    VkImage* Images;
    VkImageView* ImageViews;
} FVulkanSwapchain;

typedef struct FVulkanContext
{
    VkInstance Instance;
    VkAllocationCallbacks* Allocator;
    VkSurfaceKHR Surface;
    VkDebugUtilsMessengerEXT DebugMessenger;

    FVulkanDevice Device;

    /** The framebuffer's current width. */
    uint32 FrameBufferWidth;
    
    /** The framebuffer's current height. */
    uint32 FrameBufferHeight;

    FVulkanSwapchain Swapchain;
    uint32 ImageIndex;
    uint32 CurrentFrame;

    bool8 bRecreatingSwapchain;
} FVulkanContext;