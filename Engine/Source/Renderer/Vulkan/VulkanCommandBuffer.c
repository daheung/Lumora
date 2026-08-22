#include "VulkanCommandBuffer.h"
#include "Core/HAL/LumoraMemory.h"

void VulkanAllocateCommandBuffer(FVulkanContext* VulkanContext, VkCommandPool Pool, bool8 bIsPrimary, FVulkanCommandBuffer* OutCommandBuffer)
{
	HZeroMemory(OutCommandBuffer, sizeof(OutCommandBuffer));

	VkCommandBufferAllocateInfo AllocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	AllocateInfo.commandPool = Pool;
	AllocateInfo.level = bIsPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	AllocateInfo.commandBufferCount = 1;
	AllocateInfo.pNext = NULL;

	OutCommandBuffer->State = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
	VK_CHECK(vkAllocateCommandBuffers(VulkanContext->Device.Device, &AllocateInfo, &OutCommandBuffer->Handle));
	OutCommandBuffer->State = COMMAND_BUFFER_STATE_READY;
}

void VulkanReleaseCommandBuffer(FVulkanContext* VulkanContext, VkCommandPool Pool, FVulkanCommandBuffer* CommandBuffer)
{
	vkFreeCommandBuffers(VulkanContext->Device.Device, Pool, 1, &CommandBuffer->Handle);
	CommandBuffer->Handle = NULL;
	CommandBuffer->State = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

void VulkanBeginCommandBuffer(FVulkanCommandBuffer* CommandBuffer, bool8 bIsSingleUse, bool8 bIsRenderPassContinue, bool8 bIsSimultaneousUse)
{
	VkCommandBufferBeginInfo BeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	BeginInfo.flags = 0;
	if (bIsSingleUse)
	{
		BeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	}
	if (bIsRenderPassContinue)
	{
		BeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	}
	if (bIsSimultaneousUse)
	{
		BeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	}

	VK_CHECK(vkBeginCommandBuffer(CommandBuffer->Handle, &BeginInfo));
	CommandBuffer->State = COMMAND_BUFFER_STATE_RECORDING;
}

void VulkanEndCommandBuffer(FVulkanCommandBuffer* CommandBuffer)
{
	VK_CHECK(vkEndCommandBuffer(CommandBuffer->Handle));
	CommandBuffer->State = COMMAND_BUFFER_STATE_RECORDING_ENDED;
}

void VulkanUpdateSubmittedCommandBuffer(FVulkanCommandBuffer* CommandBuffer)
{
	CommandBuffer->State = COMMAND_BUFFER_STATE_SUBMITTED;
}

void VulkanResetCommandBuffer(FVulkanCommandBuffer* CommandBuffer)
{
	CommandBuffer->State = COMMAND_BUFFER_STATE_READY;
}

void VulkanAllocateAndBeginCommandBufferSingleUse(FVulkanContext* VulkanContext, VkCommandPool Pool, FVulkanCommandBuffer* OutCommandBuffer)
{
	VulkanAllocateCommandBuffer(VulkanContext, Pool, TRUE, OutCommandBuffer);
	VulkanBeginCommandBuffer(OutCommandBuffer, TRUE, FALSE, FALSE);
}

void VulkanEndCommandBufferSingleUse(FVulkanContext* VulkanContext, VkCommandPool Pool, FVulkanCommandBuffer* CommandBuffer, VkQueue Queue)
{
	/** End the command buffer. */
	VulkanEndCommandBuffer(CommandBuffer);

	/** Submit the queue. */
	VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &CommandBuffer->Handle;
	VK_CHECK(vkQueueSubmit(Queue, 1, &SubmitInfo, 0));

	/** Wait for it to finish. */
	VK_CHECK(vkQueueWaitIdle(Queue));

	/** Free the command buffer. */
	VulkanReleaseCommandBuffer(VulkanContext, Pool, CommandBuffer);
}

