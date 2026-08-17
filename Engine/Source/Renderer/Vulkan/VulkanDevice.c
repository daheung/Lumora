#include "VulkanDevice.h"
#include "Core/Logger.h"
#include "Core/Misc/CString.h"
#include "Core/HAL/LumoraMemory.h"
#include "Core/Containers/Array.h"

typedef struct FVulkanPhysicalDeviceRequireents
{
    const char** DeviceExtensionNames;
    bool8 bGraphics;
    bool8 bPresent;
    bool8 bCompute;
    bool8 bTransfer;
    bool8 bSamplerAnisotropy;
    bool8 bDiscreteGpu;
} FVulkanPhysicalDeviceRequireents;

typedef struct FVulkanPhysicalDeviceQueueFamilyInfo
{
    uint32 GraphicsFamilyIndex;
    uint32 PresentFamilyIndex;
    uint32 ComputeFamilyIndex;
    uint32 TransferFamilyIndex;
} FVulkanPhysicalDeviceQueueFamilyInfo;

bool8 SelectPhysicalDevice(FVulkanContext* VulkanContext);
bool8 PhysicalDeviceMeetsRequirements(
    VkPhysicalDevice Device,
    VkSurfaceKHR Surface,
    const VkPhysicalDeviceProperties* Properties,
    const VkPhysicalDeviceFeatures* Features,
    const FVulkanPhysicalDeviceRequireents* Requirements,
    FVulkanPhysicalDeviceQueueFamilyInfo* OutQueueFamilyInfo,
    FVulkanSwapchainSupportInfo* OutSwapchainSupport
);

bool8 VulkanCreateDevice(FVulkanContext* VulkanContext)
{
    const bool8 bSelectSucceed = SelectPhysicalDevice(VulkanContext);
    if (!bSelectSucceed)
    {
        return FALSE;
    }

    LUMORA_INFO("Cretaing logical device...");

    /** NOTE: Do not create additional queues for shared indices. */
    bool8 bPresentSharesGraphicsQueue  = VulkanContext->Device.GraphicsQueueIndex == VulkanContext->Device.PresentQueueIndex;
    bool8 bTransferSharesGraphicsQueue = VulkanContext->Device.GraphicsQueueIndex == VulkanContext->Device.TransferQueueIndex;
    uint32 IndexCount = 1;
    if (!bPresentSharesGraphicsQueue)
    {
        IndexCount++;
    }
    if (!bTransferSharesGraphicsQueue)
    {
        IndexCount++;
    }

    uint32* Indices = CArrayCreateWithCapacity(sizeof(uint32), IndexCount);
    uint8 Index = 0;
    Indices[Index++] = VulkanContext->Device.GraphicsQueueIndex;
    if (!bPresentSharesGraphicsQueue)
    {
        Indices[Index++] = VulkanContext->Device.PresentQueueIndex;
    }
    if (!bTransferSharesGraphicsQueue)
    {
        Indices[Index++] = VulkanContext->Device.TransferQueueIndex;
    }

    VkDeviceQueueCreateInfo* QueueCreateInfos = CArrayCreateWithCapacity(sizeof(VkDeviceQueueCreateInfo), IndexCount);
    for (uint32 I = 0; I < IndexCount; ++I)
    {
        QueueCreateInfos[I].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueCreateInfos[I].queueFamilyIndex = Indices[I];
        QueueCreateInfos[I].queueCount = 1;

        /** TODO: Enable this for a future enhancement. */
        //if (Indices[I] == VulkanContext->Device.GraphicsQueueIndex)
        //{
        //    QueueCreateInfos[I].queueCount = 2;
        //}
        QueueCreateInfos[I].flags = 0;
        QueueCreateInfos[I].pNext = NULL;
        float32 QueuePriority = 1.0f;
        QueueCreateInfos[I].pQueuePriorities = &QueuePriority;
    }

    /** Request device features. 
     *  TODO: Should be config driven. */
    VkPhysicalDeviceFeatures DeviceFeatures = { 0 };
    DeviceFeatures.samplerAnisotropy = VK_TRUE;     // Request anistrophy
    
    VkDeviceCreateInfo DeviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    DeviceCreateInfo.queueCreateInfoCount = IndexCount;
    DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfos;
    DeviceCreateInfo.pEnabledFeatures = &DeviceFeatures;
    DeviceCreateInfo.enabledExtensionCount = 1;
    const char* ExtensionNames = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    DeviceCreateInfo.ppEnabledExtensionNames = &ExtensionNames;

    /** Deprecated and ignored, so pass nothing. */
    DeviceCreateInfo.enabledLayerCount = 0;
    DeviceCreateInfo.ppEnabledLayerNames = NULL;    

    /** Create the Vulkan device. */
    VK_CHECK(vkCreateDevice(VulkanContext->Device.PhysicalDevice, &DeviceCreateInfo, VulkanContext->Allocator, &VulkanContext->Device.Device));
    
    LUMORA_INFO("Logical device created.");

    /** Get queues. */
    vkGetDeviceQueue(VulkanContext->Device.Device, VulkanContext->Device.GraphicsQueueIndex, 0, &VulkanContext->Device.GraphicsQueue);
    vkGetDeviceQueue(VulkanContext->Device.Device, VulkanContext->Device.PresentQueueIndex, 0, &VulkanContext->Device.PresentQueue);
    vkGetDeviceQueue(VulkanContext->Device.Device, VulkanContext->Device.TransferQueueIndex, 0, &VulkanContext->Device.TransferQueue);

    LUMORA_INFO("Queues obtained.");

    CArrayRelease(QueueCreateInfos);
    CArrayRelease(Indices);

    return TRUE;
}

void VulkanReleaseDevice(FVulkanContext* VulkanContext)
{
    /** Unset queues. */
    VulkanContext->Device.GraphicsQueue = NULL;
    VulkanContext->Device.PresentQueue  = NULL;
    VulkanContext->Device.TransferQueue = NULL;

    /** Destroy logical device. */
    LUMORA_INFO("Destroying logical device...");
    if (VulkanContext->Device.Device)
    {
        vkDestroyDevice(VulkanContext->Device.Device, VulkanContext->Allocator);
        VulkanContext->Device.Device = NULL;
    }

    /** Physical devices are not destroyed. */
    LUMORA_INFO("Releaseing physical device resources...");
    VulkanContext->Device.PhysicalDevice = NULL;

    if (VulkanContext->Device.SwapchainSupport.Formats)
    {
        const uint32 SwapchainFormatsAllocSize = sizeof(VkSurfaceFormatKHR) * VulkanContext->Device.SwapchainSupport.FormatCount;
        HFree(VulkanContext->Device.SwapchainSupport.Formats, SwapchainFormatsAllocSize, MEMORY_TAG_RENDERER);
        VulkanContext->Device.SwapchainSupport.Formats = NULL;
        VulkanContext->Device.SwapchainSupport.FormatCount = 0;
    }

    if (VulkanContext->Device.SwapchainSupport.PresentModes)
    {
        const uint32 PresentModeAllocSize = sizeof(VkPresentModeKHR) * VulkanContext->Device.SwapchainSupport.PresentModeCount;
        HFree(VulkanContext->Device.SwapchainSupport.PresentModes, PresentModeAllocSize, MEMORY_TAG_RENDERER);
        VulkanContext->Device.SwapchainSupport.PresentModes = NULL;
        VulkanContext->Device.SwapchainSupport.PresentModeCount = 0;
    }

    const uint32 SwapchainCapAllocSize = sizeof(VulkanContext->Device.SwapchainSupport.Capabilities);
    HZeroMemory(&VulkanContext->Device.SwapchainSupport.Capabilities, SwapchainCapAllocSize);

    VulkanContext->Device.GraphicsQueueIndex = -1;
    VulkanContext->Device.PresentQueueIndex  = -1;
    VulkanContext->Device.PresentQueueIndex  = -1;
}

void VulkanDeviceQuerySwapchainSupport(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface, FVulkanSwapchainSupportInfo* OutSupportInfo)
{
    /** Surface capabilities */
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &OutSupportInfo->Capabilities));

    /** Surface formats */
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &OutSupportInfo->FormatCount, NULL));

    if (OutSupportInfo->FormatCount != 0)
    {
        if (!OutSupportInfo->Formats)
        {
            const uint32 FormatsAllocSize = sizeof(VkSurfaceFormatKHR) * OutSupportInfo->FormatCount;
            OutSupportInfo->Formats = HAllocate(FormatsAllocSize, MEMORY_TAG_RENDERER);
        }

        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &OutSupportInfo->FormatCount, OutSupportInfo->Formats));
    }

    /** Present modes */
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &OutSupportInfo->PresentModeCount, NULL));
    if (OutSupportInfo->PresentModeCount != 0)
    {
        if (!OutSupportInfo->PresentModes)
        {
            const uint32 PresentAllocSize = sizeof(VkPresentModeKHR) * OutSupportInfo->PresentModeCount;
            OutSupportInfo->PresentModes = HAllocate(PresentAllocSize, MEMORY_TAG_RENDERER);
        }

        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &OutSupportInfo->PresentModeCount, OutSupportInfo->PresentModes));
    }
}

bool8 SelectPhysicalDevice(FVulkanContext* VulkanContext)
{
    uint32 PhysicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(VulkanContext->Instance, &PhysicalDeviceCount, NULL));
    if (PhysicalDeviceCount == 0)
    {
        LUMORA_FATAL("No devices which suppoert Vulkan were found.");
        return FALSE;
    }

    VkPhysicalDevice* PhysicalDevices = CArrayCreateWithCapacity(sizeof(VkPhysicalDevice), PhysicalDeviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(VulkanContext->Instance, &PhysicalDeviceCount, PhysicalDevices));
    for (uint32 I = 0; I < PhysicalDeviceCount; ++I)
    {
        VkPhysicalDeviceProperties DeviceProperties = { 0 };
        vkGetPhysicalDeviceProperties(PhysicalDevices[I], &DeviceProperties);

        VkPhysicalDeviceFeatures DeviceFeatures = { 0 };
        vkGetPhysicalDeviceFeatures(PhysicalDevices[I], &DeviceFeatures);

        VkPhysicalDeviceMemoryProperties MemoryProperties = { 0 };
        vkGetPhysicalDeviceMemoryProperties(PhysicalDevices[I], &MemoryProperties);

        /** TODO: These requirements should probably be driven by engine configuration. */
        FVulkanPhysicalDeviceRequireents Requirements = { 0 };
        Requirements.bGraphics = TRUE;
        Requirements.bPresent = TRUE;
        Requirements.bTransfer = TRUE;
        /** NOTE: Enable this if compute will be required. */
        //Requirements.bCompute = TRUE;
        Requirements.bSamplerAnisotropy = TRUE;
        Requirements.bDiscreteGpu = TRUE;
        Requirements.DeviceExtensionNames = CArrayCreate(sizeof(const char*));
         
        const char* SwapchainExtensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        CArrayPush(Requirements.DeviceExtensionNames, &SwapchainExtensionName);

        FVulkanPhysicalDeviceQueueFamilyInfo QueueFamilyInfo = { 0 };
        bool8 bSupportsRequirements = PhysicalDeviceMeetsRequirements(
            PhysicalDevices[I], 
            VulkanContext->Surface, 
            &DeviceProperties, 
            &DeviceFeatures, 
            &Requirements, 
            &QueueFamilyInfo, 
            &VulkanContext->Device.SwapchainSupport
        );
        if (bSupportsRequirements)
        {
            LUMORA_INFO("Selected device: '%s'.", DeviceProperties.deviceName);

            /** GPU type, etc. */
            switch (DeviceProperties.deviceType)
            {
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                LUMORA_INFO("GPU type is Unknown.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                LUMORA_INFO("GPU type is Intergrated.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                LUMORA_INFO("GPU type is Discrete.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                LUMORA_INFO("GPU type is Virtual.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                LUMORA_INFO("GPU type is CPU.");
                break;
            }

            const uint32 DriverMajor = VK_VERSION_MAJOR(DeviceProperties.driverVersion);
            const uint32 DriverMinor = VK_VERSION_MINOR(DeviceProperties.driverVersion);
            const uint32 DriverPatch = VK_VERSION_PATCH(DeviceProperties.driverVersion);
            LUMORA_INFO("GPU Driver version: %d.%d.%d", DriverMajor, DriverMinor, DriverPatch);

            const uint32 ApiMajor = VK_VERSION_MAJOR(DeviceProperties.apiVersion);
            const uint32 ApiMinor = VK_VERSION_MINOR(DeviceProperties.apiVersion);
            const uint32 ApiPatch = VK_VERSION_PATCH(DeviceProperties.apiVersion);
            LUMORA_INFO("Vulkan API version: %d.%d.%d", ApiMajor, ApiMinor, ApiPatch);

            /** Memory information */
            for (uint32 J = 0; J < MemoryProperties.memoryHeapCount; ++J)
            {
                float32 MemorySizeGib = (((float32)MemoryProperties.memoryHeaps[J].size) / 1024.0f / 1024.0f / 1024.0f);
                if (MemoryProperties.memoryHeaps[J].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                {
                    LUMORA_INFO("Local GPU Memory: %2.f GiB", MemorySizeGib);
                }
                else
                {
                    LUMORA_INFO("Shared System memory: %2.f GiB", MemorySizeGib);
                }
            }

            VulkanContext->Device.PhysicalDevice = PhysicalDevices[I];
            VulkanContext->Device.GraphicsQueueIndex = (int32)QueueFamilyInfo.GraphicsFamilyIndex;
            VulkanContext->Device.PresentQueueIndex  = (int32)QueueFamilyInfo.PresentFamilyIndex;
            VulkanContext->Device.TransferQueueIndex = (int32)QueueFamilyInfo.TransferFamilyIndex;
            /** NOTE: set compute index here if needed. */

            /** Keep a copy of properties, features and memory info for later use. */
            VulkanContext->Device.Properties = DeviceProperties;
            VulkanContext->Device.Features = DeviceFeatures;
            VulkanContext->Device.Memory = MemoryProperties;
            break;
        }
    }


    /** Ensure a device was selected. */
    if (!VulkanContext->Device.PhysicalDevice)
    {
        LUMORA_ERROR("No Physical devices were found which meet the requirements.");
        return FALSE;
    }

    //CArrayRelease(PhysicalDevices);

    LUMORA_INFO("Physical device selected.");
    return TRUE;
}

bool8 PhysicalDeviceMeetsRequirements(
    VkPhysicalDevice Device, 
    VkSurfaceKHR Surface, 
    const VkPhysicalDeviceProperties* Properties, 
    const VkPhysicalDeviceFeatures* Features, 
    const FVulkanPhysicalDeviceRequireents* Requirements, 
    FVulkanPhysicalDeviceQueueFamilyInfo* OutQueueFamilyInfo, 
    FVulkanSwapchainSupportInfo* OutSwapchainSupport
) {

    /** Evaluate device properties to determine if it meets the needs of out appliction. */
    OutQueueFamilyInfo->GraphicsFamilyIndex = (uint32)-1;
    OutQueueFamilyInfo->PresentFamilyIndex  = (uint32)-1;
    OutQueueFamilyInfo->ComputeFamilyIndex  = (uint32)-1;
    OutQueueFamilyInfo->TransferFamilyIndex = (uint32)-1;

    /** Discrete GPU? */
    if (Requirements->bDiscreteGpu)
    {
        if (Properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            LUMORA_INFO("Device is not a discrete GPU, and ont is required. Skipping.");
            return FALSE;
        }
    }

    uint32 QueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, 0);
    VkQueueFamilyProperties* QueueFamilies = CArrayCreateWithCapacity(sizeof(VkQueueFamilyProperties), QueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, QueueFamilies);

    LUMORA_INFO("Graphics | Present | Compute | Transfer | Name");
    uint8 MinTransferScore = 255;
    for (uint32 Index = 0; Index < QueueFamilyCount; ++Index)
    {
        uint8 CurrentTransferScore = 0;

        /** Graphics queue? */
        if (QueueFamilies[Index].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            OutQueueFamilyInfo->GraphicsFamilyIndex = 1;
            ++CurrentTransferScore;
        }

        /** Compute queue? */
        if (QueueFamilies[Index].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            OutQueueFamilyInfo->ComputeFamilyIndex = 1;
            ++CurrentTransferScore;
        }

        /** Transfer queue? */
        if (QueueFamilies[Index].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            /** 
             * Take the intdex if it is the current lowset. This increases the 
             * likeihood that it is a dedicated transfer queue.
             */
            if (CurrentTransferScore <= MinTransferScore)
            {
                MinTransferScore = CurrentTransferScore;
                OutQueueFamilyInfo->TransferFamilyIndex = 1;
            }
        }

        /** Present queue? */
        VkBool32 bSupportsPresent = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(Device, Index, Surface, &bSupportsPresent));
        if (bSupportsPresent)
        {
            OutQueueFamilyInfo->PresentFamilyIndex = 0;
        }
    }

    /** Print out some info about the device */
    LUMORA_INFO(
        "       %d |        %d |       %d |       %d | %s",
        OutQueueFamilyInfo->GraphicsFamilyIndex != -1,
        OutQueueFamilyInfo->PresentFamilyIndex  != -1,
        OutQueueFamilyInfo->ComputeFamilyIndex  != -1,
        OutQueueFamilyInfo->TransferFamilyIndex != -1,
        Properties->deviceName
    );
        
    const bool8 bSatisfiesGraphics = (!Requirements->bGraphics || (Requirements->bGraphics && OutQueueFamilyInfo->GraphicsFamilyIndex != -1));
    const bool8 bSatisfiesPresent  = (!Requirements->bPresent  || (Requirements->bPresent  && OutQueueFamilyInfo->PresentFamilyIndex  != -1));
    const bool8 bSatisfiesCompute  = (!Requirements->bCompute  || (Requirements->bCompute  && OutQueueFamilyInfo->ComputeFamilyIndex  != -1));
    const bool8 bSatisfiesTransfer = (!Requirements->bTransfer || (Requirements->bTransfer && OutQueueFamilyInfo->TransferFamilyIndex != -1));
    if (bSatisfiesGraphics && bSatisfiesPresent && bSatisfiesCompute && bSatisfiesTransfer)
    {
        LUMORA_INFO("Device meets queue requirements.");
        LUMORA_TRACE("Graphics Family Index: %i", OutQueueFamilyInfo->GraphicsFamilyIndex);
        LUMORA_TRACE("Present Family Index: %i", OutQueueFamilyInfo->PresentFamilyIndex);
        LUMORA_TRACE("Transfer Family Index: %i", OutQueueFamilyInfo->TransferFamilyIndex);
        LUMORA_TRACE("Compute Family Index: %i", OutQueueFamilyInfo->ComputeFamilyIndex);

        /** Query swapchain support. */
        VulkanDeviceQuerySwapchainSupport(Device, Surface, OutSwapchainSupport);

        if (OutSwapchainSupport->FormatCount < 1 || OutSwapchainSupport->PresentModeCount < 1)
        {
            if (OutSwapchainSupport->Formats)
            {
                const uint32 FormatsAllocSize = sizeof(VkSurfaceFormatKHR) * OutSwapchainSupport->FormatCount;
                HFree(OutSwapchainSupport->Formats, FormatsAllocSize, MEMORY_TAG_RENDERER);
            }

            if (OutSwapchainSupport->PresentModes)
            {
                const uint32 PresentAllocSize = sizeof(VkPresentModeKHR) * OutSwapchainSupport->PresentModeCount;
                HFree(OutSwapchainSupport->PresentModes, PresentAllocSize, MEMORY_TAG_RENDERER);
            }

            LUMORA_INFO("Required swapchain support not present, skipping device.");
            return FALSE;
        }

        /** Device extensions. */
        if (Requirements->DeviceExtensionNames)
        {
            uint32 AvailableExtensionCount = 0;
            VkExtensionProperties* AvailableExtensions = NULL;
            VK_CHECK(vkEnumerateDeviceExtensionProperties(Device, NULL, &AvailableExtensionCount, NULL));

            if (AvailableExtensionCount != 0)
            {
                const uint32 ExtensionsAllocSize = sizeof(VkExtensionProperties) * AvailableExtensionCount;
                AvailableExtensions = HAllocate(ExtensionsAllocSize, MEMORY_TAG_RENDERER);
                VK_CHECK(vkEnumerateDeviceExtensionProperties(Device, NULL, &AvailableExtensionCount, AvailableExtensions));

                uint32 RequiredExtensionCount = (uint32)CArrayLength(Requirements->DeviceExtensionNames);
                for (uint32 I = 0; I < RequiredExtensionCount; ++I)
                {
                    bool8 bFound = FALSE;
                    for (uint32 J = 0; J < AvailableExtensionCount; ++J)
                    {
                        if (Strcmp(Requirements->DeviceExtensionNames[I], AvailableExtensions[J].extensionName) == 0)
                        {
                            bFound = TRUE;
                            break;
                        }
                    }

                    if (!bFound)
                    {
                        LUMORA_INFO("Required extension not found: '%s', skipping device.", Requirements->DeviceExtensionNames[I]);
                        HFree(AvailableExtensions, ExtensionsAllocSize, MEMORY_TAG_RENDERER);
                        return FALSE;
                    }
                }

                HFree(AvailableExtensions, ExtensionsAllocSize, MEMORY_TAG_RENDERER);
            }
        }

        /** Samplar anisotropy */
        if (Requirements->bSamplerAnisotropy && !Features->samplerAnisotropy)
        {
            LUMORA_INFO("Device does not support samplerAnisotropy, skipping.");
            return FALSE;
        }

        /** Device meets all requirements. */
        return TRUE;
    }

    CArrayRelease(QueueFamilies);
    return FALSE;
}
