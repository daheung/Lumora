#include "Vulkan/VulkanBuffer.h"

#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanCommandBuffer.h"
#include "Vulkan/VulkanUtils.h"

#include "Core/HAL/LumoraMemory.h"
#include "Core/Logger.h"

bool8 VulkanCreateBuffer(FVulkanContext* VulkanContext, size_t Size, VkBufferUsageFlagBits Usage, uint32 MemoryPropertyFlags, bool8 bBindOnCreate, FVulkanBuffer* OutBuffer)
{
    HZeroMemory(OutBuffer, sizeof(FVulkanBuffer));
    OutBuffer->TotalSize = Size;
    OutBuffer->Usage = Usage;
    OutBuffer->MemoryPropertyFlags = MemoryPropertyFlags;

    VkBufferCreateInfo BufferCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    BufferCreateInfo.size = Size;
    BufferCreateInfo.usage = Usage;
    BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;   // NOTE: Only used in one queue.

    VK_CHECK(vkCreateBuffer(VulkanContext->Device.Device, &BufferCreateInfo, VulkanContext->Allocator, &OutBuffer->Handle));

    /** Gather memory requirements. */
    VkMemoryRequirements Requirements = { 0 };
    vkGetBufferMemoryRequirements(VulkanContext->Device.Device, OutBuffer->Handle, &Requirements);
    OutBuffer->MemoryIndex = VulkanContext->FindMemoryIndexFunc(Requirements.memoryTypeBits, OutBuffer->MemoryPropertyFlags);
    if (OutBuffer->MemoryIndex == -1)
    {
        LUMORA_ERROR("Unable to create vuklan buffer due to required memory type index was not found.");
        return FALSE;
    }

    /** Allocate memory info */
    VkMemoryAllocateInfo AllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    AllocateInfo.allocationSize = Requirements.size;
    AllocateInfo.memoryTypeIndex = (uint32)OutBuffer->MemoryIndex;

    /** Allocate the memory. */
    VkResult Result = vkAllocateMemory(VulkanContext->Device.Device, &AllocateInfo, VulkanContext->Allocator, &OutBuffer->Memory);
    if (Result != VK_SUCCESS)
    {
        LUMORA_ERROR("Unable to create vulkan buffer due to required memory allocatin failed. Error: %i", Result);
        return FALSE;
    }

    if (bBindOnCreate)
    {
        VulkanBufferBind(VulkanContext, OutBuffer, 0);
    }

    return TRUE;
}

void VulkanReleaseBuffer(FVulkanContext* VulkanContext, FVulkanBuffer* Buffer)
{
    if (Buffer->Memory)
    {
        vkFreeMemory(VulkanContext->Device.Device, Buffer->Memory, VulkanContext->Allocator);
        Buffer->Memory = NULL;
    }
    if (Buffer->Handle)
    {
        vkDestroyBuffer(VulkanContext->Device.Device, Buffer->Handle, VulkanContext->Allocator);
        Buffer->Handle = NULL;
    }
}

bool8 VulkanBufferResize(FVulkanContext* VulkanContext, size_t NewSize, FVulkanBuffer* Buffer, VkQueue Queue, VkCommandPool Pool)
{
    /** Create new buffer. */
    VkBufferCreateInfo BufferCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    BufferCreateInfo.size = NewSize;
    BufferCreateInfo.usage = Buffer->Usage;
    BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;   // NOTE: Only used in one queue.

    VkBuffer NewBuffer = NULL;
    VK_CHECK(vkCreateBuffer(VulkanContext->Device.Device, &BufferCreateInfo, VulkanContext->Allocator, &NewBuffer));

    /** Gather memory requirements. */
    VkMemoryRequirements Requirements = { 0 };
    vkGetBufferMemoryRequirements(VulkanContext->Device.Device, NewBuffer, &Requirements);

    /** Allocate memory info */
    VkMemoryAllocateInfo AllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    AllocateInfo.allocationSize = Requirements.size;
    AllocateInfo.memoryTypeIndex = (uint32)Buffer->MemoryIndex;

    /** Allocate the memory. */
    VkDeviceMemory NewMemory = NULL;
    VkResult Result = vkAllocateMemory(VulkanContext->Device.Device, &AllocateInfo, VulkanContext->Allocator, &NewMemory);
    if (Result != VK_SUCCESS)
    {
        LUMORA_ERROR("Unable to resize vulkan buffer due to required memory allocatin failed. Error: %i", Result);
        return FALSE;
    }

    /** Bind the new buffer's memory. */
    VK_CHECK(vkBindBufferMemory(VulkanContext->Device.Device, NewBuffer, NewMemory, 0));

    /** Copy over the data. */
    VulkanBufferCopyTo(VulkanContext, Pool, NULL, Queue, Buffer->Handle, 0, NewBuffer, 0, Buffer->TotalSize);

    /** Make sure anything potentially using these is finishing. */
    vkDeviceWaitIdle(VulkanContext->Device.Device);     //   TODO: use fence

    /** Destroy the old */
    if (Buffer->Memory)
    {
        vkFreeMemory(VulkanContext->Device.Device, Buffer->Memory, VulkanContext->Allocator);
        Buffer->Memory = NULL;
    }
    if (Buffer->Handle)
    {
        vkDestroyBuffer(VulkanContext->Device.Device, Buffer->Handle, VulkanContext->Allocator);
        Buffer->Handle = NULL;
    }

    /** Set new properties */
    Buffer->TotalSize = NewSize;
    Buffer->Handle = NewBuffer;
    Buffer->Memory = NewMemory;

    return TRUE;
}

void VulkanBufferBind(FVulkanContext* VulkanContext, FVulkanBuffer* Buffer, size_t Offset)
{
    VK_CHECK(vkBindBufferMemory(VulkanContext->Device.Device, Buffer->Handle, Buffer->Memory, Offset));
}

void* VulkanBufferLockMemory(FVulkanContext* VulkanContext, FVulkanBuffer* Buffer, size_t Offset, size_t Size, uint32 Flags)
{
    void* Data = NULL;
    VK_CHECK(vkMapMemory(VulkanContext->Device.Device, Buffer->Memory, Offset, Size, Flags, &Data));
    return Data;
}

void VulkanBufferUnlockMemory(FVulkanContext* VulkanContext, FVulkanBuffer* Buffer)
{
    vkUnmapMemory(VulkanContext->Device.Device, Buffer->Memory);
}

void VulkanBufferLoadData(FVulkanContext* VulkanContext, FVulkanBuffer* Buffer, size_t Offset, size_t Size, uint32 Flags, const void* Data)
{
    void* DataPtr = NULL;
    VK_CHECK(vkMapMemory(VulkanContext->Device.Device, Buffer->Memory, Offset, Size, Flags, &DataPtr));
    HCopyMemory(DataPtr, Data, Size);
    vkUnmapMemory(VulkanContext->Device.Device, Buffer->Memory);
}

void VulkanBufferCopyTo(FVulkanContext* VulkanContext, VkCommandPool Pool, VkFence Fence, VkQueue Queue, VkBuffer Source, size_t SourceOffset, VkBuffer Dest, size_t DestOffset, size_t Size)
{
    LUMORA_UNUSED_PARAM(Fence);

    vkQueueWaitIdle(Queue);

    /** Create a one-time-use command buffer. */
    FVulkanCommandBuffer TempCommandBuffer = { 0 };
    VulkanAllocateAndBeginCommandBufferSingleUse(VulkanContext, Pool, &TempCommandBuffer);

    /** Prepare the copy command and add it to the command buffer. */
    VkBufferCopy CopyRegion = { 0 };
    CopyRegion.srcOffset = SourceOffset;
    CopyRegion.dstOffset = DestOffset;
    CopyRegion.size = Size;

    vkCmdCopyBuffer(TempCommandBuffer.Handle, Source, Dest, 1, &CopyRegion);

    /** submit the buffer for execution and wait for it to complete. */
    VulkanEndCommandBufferSingleUse(VulkanContext, Pool, &TempCommandBuffer, Queue);
}
