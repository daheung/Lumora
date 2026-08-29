#pragma once

#include "Vulkan/VulkanTypes.inl"

bool8 VulkanCreateBuffer(FVulkanContext* VulkanContext, size_t Size, VkBufferUsageFlagBits Usage, uint32 MemoryPropertyFlags, bool8 bBindOnCreate, FVulkanBuffer* OutBuffer);

void VulkanReleaseBuffer(FVulkanContext* VulkanContext, FVulkanBuffer* Buffer);

bool8 VulkanBufferResize(FVulkanContext* VulkanContext, size_t NewSize, FVulkanBuffer* Buffer, VkQueue Queue, VkCommandPool Pool);

void VulkanBufferBind(FVulkanContext* VulkanContext, FVulkanBuffer* Buffer, size_t Offset);

void* VulkanBufferLockMemory(FVulkanContext* VulkanContext, FVulkanBuffer* Buffer, size_t Offset, size_t Size, uint32 Flags);

void VulkanBufferUnlockMemory(FVulkanContext* VulkanContext, FVulkanBuffer* Buffer);

void VulkanBufferLoadData(FVulkanContext* VulkanContext, FVulkanBuffer* Buffer, size_t Offset, size_t Size, uint32 Flags, const void* Data);

void VulkanBufferCopyTo(FVulkanContext* VulkanContext, VkCommandPool Pool, VkFence Fence, VkQueue Queue, VkBuffer Source, size_t SourceOffset, VkBuffer Dest, size_t DestOffset, size_t Size);