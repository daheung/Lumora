#include "Vulkan/Shaders/VulkanObjectShader.h"
#include "Vulkan/Shaders/VulkanShaderUtils.h"
#include "Vulkan/VulkanPipeline.h"
#include "Core/Logger.h"
#include "Core/Math/MathFwd.h"

#define BUILTIN_SHADER_NAME_OBJECT ("Builtin.ObjectShader")

bool8 VulkanCreateObjectShader(FVulkanContext* VulkanContext, FVulkanObjectShader* OutShader)
{
    /** Shader module init per stage. */
    char StageTypeStrs[OBJECT_SHADER_STAGE_COUNT][5] = { "vert", "frag" };
    VkShaderStageFlagBits StageTypes[OBJECT_SHADER_STAGE_COUNT] = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };

    for (uint32 Index = 0; Index < OBJECT_SHADER_STAGE_COUNT; ++Index)
    {
        if (!CreateShaderModule(VulkanContext, BUILTIN_SHADER_NAME_OBJECT, StageTypeStrs[Index], StageTypes[Index], Index, OutShader->Stages))
        {
            LUMORA_ERROR("Unable to create %s shader module for '%s'.", StageTypeStrs[Index], BUILTIN_SHADER_NAME_OBJECT);
            return FALSE;
        }
    }

    /** TODO: Descriptors */

    /** Pipeline creation */
    VkViewport Viewport = { 0 };
    Viewport.x        =  (float32)0.0f;
    Viewport.y        =  (float32)VulkanContext->FrameBufferHeight;
    Viewport.width    =  (float32)VulkanContext->FrameBufferWidth;
    Viewport.height   = -(float32)VulkanContext->FrameBufferHeight;
    Viewport.minDepth =  0.0f;
    Viewport.maxDepth =  1.0f;

    /** Scissor */
    VkRect2D Scissor = { 0 };
    Scissor.offset.x = 0;
    Scissor.offset.y = 0;
    Scissor.extent.width = VulkanContext->FrameBufferWidth;
    Scissor.extent.height = VulkanContext->FrameBufferHeight;

    /** Attributes */
    uint32 Offset = 0;
    const uint32 AttributeCount = 1;
    VkVertexInputAttributeDescription AttributeDescription[1] = { 0 };

    /** Position */
    VkFormat Formats[1] = {
        VK_FORMAT_R32G32B32_SFLOAT
    };

    size_t Sizes[1] = {
        sizeof(FVector3D)
    };

    for (uint32 Index = 0; Index < AttributeCount; ++Index)
    {
        AttributeDescription[Index].binding = 0;        // Binding index - should match binding description
        AttributeDescription[Index].location = Index;   // Attribute location
        AttributeDescription[Index].format = Formats[Index];
        AttributeDescription[Index].offset = Offset;
        Offset += (uint32)Sizes[Index];
    }
    
    /** Descriptor set layouts. */

    /** Stages */
    /** NOTE: Should match the number of shader->stages. */
    VkPipelineShaderStageCreateInfo StageCreateInfos[OBJECT_SHADER_STAGE_COUNT] = { 0 };
    for (uint32 Index = 0; Index < OBJECT_SHADER_STAGE_COUNT; ++Index)
    {
        StageCreateInfos[Index].sType = OutShader->Stages[Index].ShaderStageCreateInfo.sType;
        StageCreateInfos[Index] = OutShader->Stages[Index].ShaderStageCreateInfo;
    }

    const bool8 bCreatePipelineSucceed = VulkanCreateGraphicsPipeline(
        VulkanContext, 
        &VulkanContext->MainRenderPass, 
        AttributeCount,
        AttributeDescription, 
        0, 
        NULL,
        OBJECT_SHADER_STAGE_COUNT, 
        StageCreateInfos, 
        Viewport, 
        Scissor, 
        FALSE, 
        &OutShader->Pipeline
    );
    if (!bCreatePipelineSucceed)
    {
        LUMORA_ERROR("Failed to load graphics pipeline for object shader.");
        return FALSE;
    }

    return TRUE;
}

void VulkanReleaseObjectShader(FVulkanContext* VulkanContext, FVulkanObjectShader* Shader)
{
    VulkanReleasePipeline(VulkanContext, &Shader->Pipeline);

    /** Destroy shader modules. */
    for (uint32 Index = 0; Index < OBJECT_SHADER_STAGE_COUNT; ++Index)
    {
        vkDestroyShaderModule(VulkanContext->Device.Device, Shader->Stages[Index].Handle, VulkanContext->Allocator);
        Shader->Stages[Index].Handle = NULL;
    }
}

void VulkanUseObjectShader(FVulkanContext* VulkanContext, FVulkanObjectShader* Shader)
{
    uint32 ImageIndex = VulkanContext->ImageIndex;
    VulkanBindPipeline(&VulkanContext->GraphicsCommandBuffers[ImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, &Shader->Pipeline);

}
