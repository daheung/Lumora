#pragma once

#include "Vulkan/VulkanTypes.inl"

void VulkanCreateFence(FVulkanContext* VulkanContext, bool8 bCreateSignaled, FVulkanFence* OutFence);

void VulkanReleaseFence(FVulkanContext* VulkanContext, FVulkanFence* Fence);

bool8 VulkanFenceWait(FVulkanContext* VulkanContext, FVulkanFence* Fence, uint64 TimeoutNs);

void VulkanFenceReset(FVulkanContext* VulkanContext, FVulkanFence* Fence);