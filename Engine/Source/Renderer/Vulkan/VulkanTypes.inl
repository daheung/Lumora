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

    VkCommandPool GraphicsCommandPool;

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

typedef enum EVulkanRenderPassState
{
    RENDER_PASS_STATE_READY,
    RENDER_PASS_STATE_RECORDING,
    RENDER_PASS_STATE_IN_RENDER_PASS,
    RENDER_PASS_STATE_RECORDING_ENDED,
    RENDER_PASS_STATE_SUBMITTED,
    RENDER_PASS_STATE_NOT_ALLOCATED,
} EVulkanRenderPassState;

typedef struct FVulkanRenderPass
{
    VkRenderPass Handle;
    float32 X, Y, Width, Height;
    float32 Red, Green, Blue, Alpha;

    float32 Depth;
    float32 Stencil;

    EVulkanRenderPassState State;
} FVulkanRenderPass;

typedef struct FVulkanFrameBuffer
{
    VkFramebuffer Handle;
    uint32 AttachmentCount;
    VkImageView* Attachments;
    FVulkanRenderPass* RenderPass;
} FVulkanFrameBuffer;

typedef struct FVulkanSwapchain
{
    VkSurfaceFormatKHR ImageFormat;
    uint8 MaxFramesInFlight;
    VkSwapchainKHR Handle;
    uint32 ImageCount;
    VkImage* Images;
    VkImageView* ImageViews;
    FVulkanImage DepthAttachment;

    /** Framebuffers used for on-screen rendering. */
    FVulkanFrameBuffer* FrameBuffers;
} FVulkanSwapchain;

typedef enum EVulkanCommandBufferState
{
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_NOT_ALLOCATED,
} EVulkanCommandBufferState;

typedef struct FVulkanCommandBuffer
{
    VkCommandBuffer Handle;

    /** Command buffer state. */
    EVulkanCommandBufferState State;
} FVulkanCommandBuffer;

typedef struct FVulkanFence
{
    VkFence Handle;
    bool8 bIsSignaled;
} FVulkanFence;

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
    FVulkanRenderPass MainRenderPass;

    FVulkanCommandBuffer* GraphicsCommandBuffers;

    VkSemaphore* ImageAvailableSemaphores;
    VkSemaphore* QueueCompleteSemaphores;
    uint32 InFlightFenceCount;
    FVulkanFence* InFlightFences;

    /** Holds pointers to fences which exist and are owned elsewhere. */
    FVulkanFence** ImagesInFlight;

    uint32 ImageIndex;
    uint32 CurrentFrame;

    bool8 bRecreatingSwapchain;

    int32(*FindMemoryIndexFunc)(uint32 TypeFilter, uint32 PropertyFlags);
} FVulkanContext;