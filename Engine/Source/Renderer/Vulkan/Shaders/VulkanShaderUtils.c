#include "VulkanShaderUtils.h"
#include "Core/Logger.h"
#include "Core/Misc/CString.h"
#include "Core/Misc/FileSystem.h"
#include "Core/HAL/LumoraMemory.h"

bool8 CreateShaderModule(FVulkanContext* VulkanContext, const char* Name, const char* TypeStr, VkShaderStageFlagBits ShaderStageFlag, uint32 StageIndex, FVulkanShaderStage* ShaderStages)
{
    /** Build file name. */
    char FileName[512] = { 0 };
    FormatString(FileName, sizeof(FileName), "Assets/Shaders/%s.%s.spv", Name, TypeStr);

    HZeroMemory(&ShaderStages[StageIndex].CreateInfo, sizeof(VkShaderModuleCreateInfo));
    ShaderStages[StageIndex].CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    FFileHandle Handle = { 0 };
    if (!FileSystemOpen(FileName, FILE_MODE_READ, TRUE, &Handle))
    {
        LUMORA_ERROR("Unable to read shader module: %s.", FileName);
        return FALSE;
    }

    /** Read the entire file as binary. */
    size_t Size = 0;
    uint8* FileBuffer = NULL;
    if (!FileSystemReadAllBytes(&Handle, &FileBuffer, &Size))
    {
        LUMORA_ERROR("Unable to binary read shader module: %s.", FileName);
        return FALSE;
    }

    ShaderStages[StageIndex].CreateInfo.codeSize = Size;
    ShaderStages[StageIndex].CreateInfo.pCode = (uint32*)FileBuffer;

    /** Close the file. */
    FileSystemClose(&Handle);

    VK_CHECK(vkCreateShaderModule(VulkanContext->Device.Device, &ShaderStages[StageIndex].CreateInfo, NULL, &ShaderStages[StageIndex].Handle));
    
    /** Shader stage info */
    HZeroMemory(&ShaderStages[StageIndex].ShaderStageCreateInfo, sizeof(VkPipelineShaderStageCreateInfo));
    ShaderStages[StageIndex].ShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    ShaderStages[StageIndex].ShaderStageCreateInfo.stage = ShaderStageFlag;
    ShaderStages[StageIndex].ShaderStageCreateInfo.module = ShaderStages[StageIndex].Handle;
    ShaderStages[StageIndex].ShaderStageCreateInfo.pName = "main";

    if (FileBuffer)
    {
        HFree(FileBuffer, sizeof(uint8) * Size, MEMORY_TAG_STRING);
        FileBuffer = NULL;
    }

    return TRUE;
}
