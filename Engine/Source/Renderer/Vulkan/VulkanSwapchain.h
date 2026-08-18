#pragma once

#include "Vulkan/VulkanTypes.inl"


void VulkanCreateSwapchain(FVulkanContext* VulkanContext, uint32 Width, uint32 Height, FVulkanSwapchain* OutSwapchain);

void VulkanRecreateSwapchain(FVulkanContext* VulkanContext, uint32 Width, uint32 Height, FVulkanSwapchain* Swapchain);

void VulkanReleaseSwapchain(FVulkanContext* VulkanContext, FVulkanSwapchain* Swapchain);

bool8 VulkanSwapchainAcquireNextImageIndex(
	FVulkanContext* VulkanContext, 
	FVulkanSwapchain* Swapchain, 
	uint64 TimeoutNs, 
	VkSemaphore ImageAvailableSemaphore, 
	VkFence Fence, 
	uint32* OutImageIndex
);

void VulkanSwapchainPresent(
	FVulkanContext* VulkanContext,
	FVulkanSwapchain* Swapchain,
	VkQueue GraphicsQueue,
	VkQueue PresentQueue,
	VkSemaphore RenderCompleteSemaphore,
	uint32 PresentImageIndex
);