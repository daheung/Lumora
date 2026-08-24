#pragma once

#include "Vulkan/VulkanTypes.inl"

void VulkanAllocateCommandBuffer(FVulkanContext* VulkanContext, VkCommandPool Pool, bool8 bIsPrimary, FVulkanCommandBuffer* OutCommandBuffer);

void VulkanReleaseCommandBuffer(FVulkanContext* VulkanContext, VkCommandPool Pool, FVulkanCommandBuffer* CommandBuffer);

void VulkanBeginCommandBuffer(FVulkanCommandBuffer* CommandBuffer, bool8 bIsSingleUse, bool8 bIsRenderPassCountinue, bool8 bIsSimultaneousUse);

void VulkanEndCommandBuffer(FVulkanCommandBuffer* CommandBuffer);

void VulkanUpdateSubmittedCommandBuffer(FVulkanCommandBuffer* CommandBuffer);

void VulkanResetCommandBuffer(FVulkanCommandBuffer* CommandBuffer);

/** Allocates and begins recording to OutCommandBuffer. */
void VulkanAllocateAndBeginCommandBufferSingleUse(FVulkanContext* VulkanContext, VkCommandPool Pool, FVulkanCommandBuffer* OutCommandBuffer);

/** Ends recording, submits to and waits for queue operation and frees the provided command buffer. */
void VulkanEndCommandBufferSingleUse(FVulkanContext* VulkanContext, VkCommandPool Pool, FVulkanCommandBuffer* CommandBuffer, VkQueue Queue);