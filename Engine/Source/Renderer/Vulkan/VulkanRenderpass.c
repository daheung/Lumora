#include "VulkanRenderpass.h"

#include "Core/HAL/LumoraMemory.h"
#include "Core/Containers/Array.h"

void VulkanCreateRenderPass(
	FVulkanContext* VulkanContext, 
	FVulkanRenderPass* OutRenderPass, 
	float32 X, float32 Y, float32 Width, float32 Height, 
	float32 Red, float32 Green, float32 Blue, float32 Alpha, 
	float32 Depth, float32 Stencil
) {
	LUMORA_UNUSED_PARAM(X);
	LUMORA_UNUSED_PARAM(Y);
	LUMORA_UNUSED_PARAM(Width);
	LUMORA_UNUSED_PARAM(Height);
	LUMORA_UNUSED_PARAM(Red);
	LUMORA_UNUSED_PARAM(Green);
	LUMORA_UNUSED_PARAM(Blue);
	LUMORA_UNUSED_PARAM(Alpha);
	LUMORA_UNUSED_PARAM(Depth);
	LUMORA_UNUSED_PARAM(Stencil);

	/** Main subpass */
	VkSubpassDescription SubpassDesc = { 0 };
	SubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	/** Attachments TODO: make this configuration. */
	uint32 AttachmentDescCount = 2;
	VkAttachmentDescription* AttachmentDesc = CArrayCreateWithCapacity(sizeof(VkAttachmentDescription), AttachmentDescCount);

	/** Color attachment */
	VkAttachmentDescription ColorAttachmentDesc = { 0 };
	ColorAttachmentDesc.format = VulkanContext->Swapchain.ImageFormat.format;	// TODO: configurable 
	ColorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	ColorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	ColorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	ColorAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	ColorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		// Do not expected any particular layout before render pass starts.
	ColorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;	// Transitioned to after the render pass.
	ColorAttachmentDesc.flags = 0;

	CArrayPush(AttachmentDesc, &ColorAttachmentDesc);

	VkAttachmentReference ColorAttachmentReference = { 0 };
	ColorAttachmentReference.attachment = 0;	// Attachment description array index.
	ColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	SubpassDesc.colorAttachmentCount = 1;
	SubpassDesc.pColorAttachments = &ColorAttachmentReference;

	/** Depth attachment, if there is one. */
	VkAttachmentDescription DepthAttachment = { 0 };
	DepthAttachment.format = VulkanContext->Device.DepthFormat;
	DepthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	DepthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	DepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	DepthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	DepthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	CArrayPush(AttachmentDesc, &DepthAttachment);

	/** Depth attachment reference */
	VkAttachmentReference DepthAttachmentReference = { 0 };
	DepthAttachmentReference.attachment = 1;
	DepthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	
	/** TODO: Other attachment types (input, resolve, reserve) */

	/** Depth stencil data. */
	SubpassDesc.pDepthStencilAttachment = &DepthAttachmentReference;

	/** Input from a shader. */
	SubpassDesc.inputAttachmentCount = 0;
	SubpassDesc.pInputAttachments = 0;

	/** Attachments used for multi-sampling colour attachments. */
	SubpassDesc.pResolveAttachments = 0;
	
	/** Attachments not used in the subpass, but must be preserved for the next. */
	SubpassDesc.preserveAttachmentCount = 0;
	SubpassDesc.pPreserveAttachments = 0;
	
	/** Render pass dependencies. TODO: make this configurable. */
	VkSubpassDependency Dependency = { 0 };
	Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	Dependency.dstSubpass = 0;
	Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	Dependency.srcAccessMask = 0;
	Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	Dependency.dependencyFlags = 0;

	VkRenderPassCreateInfo RenderPassCreateInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	RenderPassCreateInfo.attachmentCount = AttachmentDescCount;
	RenderPassCreateInfo.pAttachments = AttachmentDesc;
	RenderPassCreateInfo.subpassCount = 1;
	RenderPassCreateInfo.pSubpasses = &SubpassDesc;
	RenderPassCreateInfo.dependencyCount = 1;
	RenderPassCreateInfo.pDependencies = &Dependency;
	RenderPassCreateInfo.pNext = 0;
	RenderPassCreateInfo.flags = 0;

	/** Create render pass. */
	VK_CHECK(vkCreateRenderPass(VulkanContext->Device.Device, &RenderPassCreateInfo, VulkanContext->Allocator, &OutRenderPass->Handle));

	CArrayRelease(AttachmentDesc);
}

void VulkanReleaseRenderPass(FVulkanContext* VulkanContext, FVulkanRenderPass* RenderPass)
{
	if (RenderPass && RenderPass->Handle)
	{
		vkDestroyRenderPass(VulkanContext->Device.Device, RenderPass->Handle, VulkanContext->Allocator);
		RenderPass->Handle = NULL;
	}
}

void VulkanRenderPassBegin(FVulkanCommandBuffer* CommandBuffer, FVulkanRenderPass* RenderPass, VkFramebuffer FrameBuffer)
{
	VkRenderPassBeginInfo BeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	BeginInfo.renderPass = RenderPass->Handle;
	BeginInfo.framebuffer = FrameBuffer;
	BeginInfo.renderArea.offset.x = RenderPass->X;
	BeginInfo.renderArea.offset.y = RenderPass->Y;
	BeginInfo.renderArea.extent.width = RenderPass->Width;
	BeginInfo.renderArea.extent.height = RenderPass->Height;

	VkClearValue ClearValue[2];
	HZeroMemory(ClearValue, sizeof(VkClearValue) * 2);
	ClearValue[0].color.float32[0] = RenderPass->Red;
	ClearValue[0].color.float32[1] = RenderPass->Green;
	ClearValue[0].color.float32[2] = RenderPass->Blue;
	ClearValue[0].color.float32[3] = RenderPass->Alpha;
	ClearValue[1].depthStencil.depth = RenderPass->Depth;
	ClearValue[1].depthStencil.stencil = RenderPass->Stencil;

	BeginInfo.clearValueCount = 2;
	BeginInfo.pClearValues = ClearValue;

	vkCmdBeginRenderPass(CommandBuffer->Handle, &BeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	CommandBuffer->State = COMMAND_BUFFER_STATE_IN_RENDER_PASS;
}

void VulkanRenderPassEnd(FVulkanCommandBuffer* CommandBuffer, FVulkanRenderPass* RenderPass)
{
	LUMORA_UNUSED_PARAM(RenderPass);
	vkCmdEndRenderPass(CommandBuffer->Handle);
	CommandBuffer->State = COMMAND_BUFFER_STATE_RECORDING;
}