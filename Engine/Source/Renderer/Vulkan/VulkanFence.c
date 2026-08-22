#include "VulkanFence.h"

#include "Core/Logger.h"

void VulkanCreateFence(FVulkanContext* VulkanContext, bool8 bCreateSignaled, FVulkanFence* OutFence)
{
    /** Make sure to signal the fence if required. */
    OutFence->bIsSignaled = bCreateSignaled;

    VkFenceCreateInfo FenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    if (OutFence->bIsSignaled)
    {
        FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    VK_CHECK(vkCreateFence(VulkanContext->Device.Device, &FenceCreateInfo, VulkanContext->Allocator, &OutFence->Handle));
}

void VulkanReleaseFence(FVulkanContext* VulkanContext, FVulkanFence* Fence)
{
    if (Fence->Handle)
    {
        vkDestroyFence(VulkanContext->Device.Device, Fence->Handle, VulkanContext->Allocator);
        Fence->Handle = NULL;
    }
    Fence->bIsSignaled = FALSE;
}

bool8 VulkanFenceWait(FVulkanContext* VulkanContext, FVulkanFence* Fence, uint64 TimeoutNs)
{
    if (!Fence->bIsSignaled)
    {
        VkResult Result = vkWaitForFences(VulkanContext->Device.Device, 1, &Fence->Handle, TRUE, TimeoutNs);
        switch (Result)
        {
        case VK_SUCCESS:
            Fence->bIsSignaled = TRUE;
            return TRUE;
        case VK_TIMEOUT:
            LUMORA_WARN("VulkanFenceWait() - Time out");
            return FALSE;
        case VK_ERROR_DEVICE_LOST:
            LUMORA_ERROR("VulkanFenceWait() - VK_ERROR_DEVICE_LIST returned.");
            return FALSE;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            LUMORA_ERROR("VulkanFenceWait() - VK_ERROR_OUT_OF_HOST_MEMORY");
            return FALSE;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            LUMORA_ERROR("VulkanFenceWait() - VK_ERROR_OUT_OF_DEVICE_MEMORY");
            return FALSE;
        default:
            LUMORA_ERROR("VulkanFenceWait() - An unknown error has occurred.");
            return FALSE;
        }
    }

    /** If already signaled, do not wait. */
    return TRUE;
}

void VulkanFenceReset(FVulkanContext* VulkanContext, FVulkanFence* Fence)
{
    if (Fence->bIsSignaled)
    {
        VK_CHECK(vkResetFences(VulkanContext->Device.Device, 1, &Fence->Handle));
        Fence->bIsSignaled = FALSE;
    }
}
