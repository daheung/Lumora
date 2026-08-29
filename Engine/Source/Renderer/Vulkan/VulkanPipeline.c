#include "Vulkan/VulkanPipeline.h"
#include "Vulkan/VulkanUtils.h"

#include "Core/HAL/LumoraMemory.h"
#include "Core/Logger.h"
#include "Core/Math/MathFwd.h"

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
) {
    /** Viewport state */
    VkPipelineViewportStateCreateInfo ViewportCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    ViewportCreateInfo.viewportCount = 1;
    ViewportCreateInfo.pViewports = &Viewport;
    ViewportCreateInfo.scissorCount = 1;
    ViewportCreateInfo.pScissors = &Scissor;

    /** Rasterizer */
    VkPipelineRasterizationStateCreateInfo RasterizationCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    RasterizationCreateInfo.depthClampEnable = VK_FALSE;
    RasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    RasterizationCreateInfo.polygonMode = bIsWireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    RasterizationCreateInfo.lineWidth = 1.0f;
    RasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    RasterizationCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    RasterizationCreateInfo.depthBiasEnable = VK_FALSE;
    RasterizationCreateInfo.depthBiasConstantFactor = 0.0f;
    RasterizationCreateInfo.depthBiasClamp = 0.0f;
    RasterizationCreateInfo.depthBiasSlopeFactor = 0.0f;

    /** Multi-sampling */
    VkPipelineMultisampleStateCreateInfo MultisamplingCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    MultisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
    MultisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    MultisamplingCreateInfo.minSampleShading = 1.0f;
    MultisamplingCreateInfo.pSampleMask = NULL;
    MultisamplingCreateInfo.alphaToCoverageEnable = VK_FALSE;
    MultisamplingCreateInfo.alphaToOneEnable = VK_FALSE;

    /** Depth and stencil testing. */
    VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    DepthStencilCreateInfo.depthTestEnable = VK_TRUE;
    DepthStencilCreateInfo.depthWriteEnable = VK_TRUE;
    DepthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    DepthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
    DepthStencilCreateInfo.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState ColorBlendAttachmentState = { 0 };
    HZeroMemory(&ColorBlendAttachmentState, sizeof(VkPipelineColorBlendAttachmentState));
    ColorBlendAttachmentState.blendEnable = VK_TRUE;
    ColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    ColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    ColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    ColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    ColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    ColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
    ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                               VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT ;

    VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    ColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    ColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    ColorBlendStateCreateInfo.attachmentCount = 1;
    ColorBlendStateCreateInfo.pAttachments = &ColorBlendAttachmentState;

    /** Dynamic state */
    const uint32 DynamicStateCount = 3;
    VkDynamicState DynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    DynamicStateCreateInfo.dynamicStateCount = DynamicStateCount;
    DynamicStateCreateInfo.pDynamicStates = DynamicStates;

    /** Vertex input */
    VkVertexInputBindingDescription BindingDescription = { 0 };
    BindingDescription.binding = 0;                             // Binding index
    BindingDescription.stride = sizeof(FVertex3D);
    BindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Move to next data entry for each vertex.

    /** Attributes */
    VkPipelineVertexInputStateCreateInfo VertexInputCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    VertexInputCreateInfo.vertexBindingDescriptionCount = 1;
    VertexInputCreateInfo.pVertexBindingDescriptions = &BindingDescription;
    VertexInputCreateInfo.vertexAttributeDescriptionCount = AttributeCount;
    VertexInputCreateInfo.pVertexAttributeDescriptions = Attributes;

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    InputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    PipelineLayoutCreateInfo.setLayoutCount = DescriptorSetLayoutCount;
    PipelineLayoutCreateInfo.pSetLayouts = DescriptorSetLayouts;

    /** Create the pipeline layout. */
    VK_CHECK(vkCreatePipelineLayout(VulkanContext->Device.Device, &PipelineLayoutCreateInfo, VulkanContext->Allocator, &OutPipeline->PipelineLayout));

    VkGraphicsPipelineCreateInfo PipelineCreateInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    PipelineCreateInfo.stageCount = StageCount;
    PipelineCreateInfo.pStages = Stages;
    PipelineCreateInfo.pVertexInputState = &VertexInputCreateInfo;
    PipelineCreateInfo.pInputAssemblyState = &InputAssemblyCreateInfo;

    PipelineCreateInfo.pViewportState = &ViewportCreateInfo;
    PipelineCreateInfo.pRasterizationState = &RasterizationCreateInfo;
    PipelineCreateInfo.pMultisampleState = &MultisamplingCreateInfo;
    PipelineCreateInfo.pDepthStencilState = &DepthStencilCreateInfo;
    PipelineCreateInfo.pColorBlendState = &ColorBlendStateCreateInfo;
    PipelineCreateInfo.pDynamicState = &DynamicStateCreateInfo;
    PipelineCreateInfo.pTessellationState = NULL;

    PipelineCreateInfo.layout = OutPipeline->PipelineLayout;

    PipelineCreateInfo.renderPass = RenderPass->Handle;
    PipelineCreateInfo.subpass = 0;
    PipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    PipelineCreateInfo.basePipelineIndex = -1;
    
    VkResult Result = vkCreateGraphicsPipelines(VulkanContext->Device.Device, VK_NULL_HANDLE, 1, &PipelineCreateInfo, VulkanContext->Allocator, &OutPipeline->Handle);
    if (IsVulkanResultSuccess(Result))
    {
        LUMORA_DEBUG("Graphics pipeline created.");
        return TRUE;
    }

    LUMORA_ERROR("VkCreateGraphicsPipelines() failed with %s.", VulkanResultString(Result, TRUE));
    return FALSE;
}

void VulkanReleasePipeline(FVulkanContext* VulkanContext, FVulkanPipeline* Pipeline)
{
    if (!Pipeline)
    {
        return;
    }

    /** Destroy pipeline */
    if (Pipeline->Handle)
    {
        vkDestroyPipeline(VulkanContext->Device.Device, Pipeline->Handle, VulkanContext->Allocator);
        Pipeline->Handle = NULL;
    }

    /** Destroy layout */
    if (Pipeline->PipelineLayout)
    {
        vkDestroyPipelineLayout(VulkanContext->Device.Device, Pipeline->PipelineLayout, VulkanContext->Allocator);
        Pipeline->PipelineLayout = NULL;
    }
}

void VulkanBindPipeline(FVulkanCommandBuffer* CommandBuffer, VkPipelineBindPoint BindPoint, FVulkanPipeline* Pipeline)
{
    vkCmdBindPipeline(CommandBuffer->Handle, BindPoint, Pipeline->Handle);
}
