#pragma once

#include "Vulkan/VulkanTypes.inl"

bool8 VulkanCreateGraphicsPipeline(
	FVulkanContext* VulkanContext,
	FVulkanRenderPass* RenderPass,
	uint32 AttributeCount,
	VkVertexInputAttributeDescription* Attributes,
	uint32 DescriptorSetLayoutCount,
	VkDescriptorSetLayout* DescriptorSetLayouts,
	uint32 StageCount,
	VkPipelineShaderStageCreateInfo* Stages,
	VkViewport Viewport,
	VkRect2D Scissor,
	bool8 bIsWireframe,
	FVulkanPipeline* OutPipeline
);

void VulkanReleasePipeline(FVulkanContext* VulkanContext, FVulkanPipeline* Pipeline);

void VulkanBindPipeline(FVulkanCommandBuffer* CommandBuffer, VkPipelineBindPoint BindPoint, FVulkanPipeline* Pipeline);