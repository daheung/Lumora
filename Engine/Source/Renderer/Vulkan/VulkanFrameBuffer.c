#include "VulkanFrameBuffer.h"
#include "Core/HAL/LumoraMemory.h"

void VulkanCreateFrameBuffer(
	FVulkanContext* VulkanContext, 
	FVulkanRenderPass* RenderPass, 
	uint32 Width, uint32 Height, 
	uint32 AttachmentCount, 
	VkImageView* Attachments, 
	FVulkanFrameBuffer* OutFrameBuffer
) {
	/** Take a copy of the attachments, render-pass and attachment count. */
	OutFrameBuffer->Attachments = HAllocate(sizeof(VkImageView) * AttachmentCount, MEMORY_TAG_RENDERER);
	for (uint32 Index = 0; Index < AttachmentCount; ++Index)
	{
		OutFrameBuffer->Attachments[Index] = Attachments[Index];
	}

	OutFrameBuffer->RenderPass = RenderPass;
	OutFrameBuffer->AttachmentCount = AttachmentCount;

	/** Create frame buffer info. */
	VkFramebufferCreateInfo FrameBufferCreateInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	FrameBufferCreateInfo.renderPass = RenderPass->Handle;
	FrameBufferCreateInfo.attachmentCount = AttachmentCount;
	FrameBufferCreateInfo.pAttachments = OutFrameBuffer->Attachments;
	FrameBufferCreateInfo.width = Width;
	FrameBufferCreateInfo.height = Height;
	FrameBufferCreateInfo.layers = 1;

	VK_CHECK(vkCreateFramebuffer(VulkanContext->Device.Device, &FrameBufferCreateInfo, VulkanContext->Allocator, &OutFrameBuffer->Handle));
}

void VulkanReleaseFrameBuffer(FVulkanContext* VulkanContext, FVulkanFrameBuffer* FrameBuffer)
{
	vkDestroyFramebuffer(VulkanContext->Device.Device, FrameBuffer->Handle, VulkanContext->Allocator);
	if (FrameBuffer->Attachments)
	{
		const uint64 FrameBufferSize = sizeof(VkImageView) * FrameBuffer->AttachmentCount;
		HFree(FrameBuffer->Attachments, FrameBufferSize, MEMORY_TAG_RENDERER);
		FrameBuffer->Attachments = NULL;
	}

	FrameBuffer->Handle = NULL;
	FrameBuffer->AttachmentCount = 0;
	FrameBuffer->RenderPass = NULL;
}
