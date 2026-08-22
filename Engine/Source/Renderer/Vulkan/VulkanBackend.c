#include "VulkanBackend.h"

#include "VulkanTypes.inl"
#include "Core/Logger.h"
#include "Core/Misc/CString.h"
#include "Core/Containers/Array.h"
#include "Core/Platform/Platform.h"
#include "Core/HAL/LumoraMemory.h"
#include "Core/Launch/LaunchEngine.h"

#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanSwapchain.h"
#include "Vulkan/VulkanRenderpass.h"
#include "Vulkan/VulkanCommandBuffer.h"
#include "Vulkan/VulkanFrameBuffer.h"
#include "Vulkan/VulkanFence.h"

struct FPlatformState;

/** static Vulkan context. */
static FVulkanContext GVulkanContext;
static uint32 GCachedFrameBufferWidth = 0;
static uint32 GCachedFrameBufferHeight = 0;

VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT MessageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
    void* UserData
);

int32 FindMemoryIndex(uint32 TypeFilter, uint32 PropertyFlags);

void CreateCommandBuffer(FRendererBackend* Backend);

void RegenerateFrameBuffers(FRendererBackend* Backend, FVulkanSwapchain* Swapchain, FVulkanRenderPass* RenderPass);

bool8 VulkanInitializeRendererBackend(FRendererBackend* Backend, const char* ApplicationName, struct FPlatformState* PlatformState)
{
    /** Function pointers */
    GVulkanContext.FindMemoryIndexFunc = FindMemoryIndex;

    /** TODO: Custom Allocator. */
    GVulkanContext.Allocator = NULL;

    ApplicationGetFrameBufferSize(&GCachedFrameBufferWidth, &GCachedFrameBufferHeight);
    GVulkanContext.FrameBufferWidth  = (GCachedFrameBufferWidth  != 0) ? GCachedFrameBufferWidth  : 1280;
    GVulkanContext.FrameBufferHeight = (GCachedFrameBufferHeight != 0) ? GCachedFrameBufferHeight : 720;
    GCachedFrameBufferWidth  = 0;
    GCachedFrameBufferHeight = 0;

    VkApplicationInfo ApplicationInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    ApplicationInfo.apiVersion = VK_API_VERSION_1_4;
    ApplicationInfo.pApplicationName = ApplicationName;
    ApplicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    ApplicationInfo.pEngineName = "Lumora Engine";
    ApplicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo CreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    CreateInfo.pApplicationInfo = &ApplicationInfo;

    /** Obtain a list of required extensions. */
    const char** RequiredExtensions = CArrayCreate(sizeof(const char*));
    const char* SurfaceExtensionName = VK_KHR_SURFACE_EXTENSION_NAME;
    CArrayPush(RequiredExtensions, &SurfaceExtensionName);  // Generic surface extension
    PlatformGetRequiredExtensionNames(&RequiredExtensions); // Platform-specific extension(s)

#if defined(_DEBUG) || defined(D_DEBUG)
    const char* ExtensionName = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    CArrayPush(RequiredExtensions, &ExtensionName); // Debug utilities

    LUMORA_DEBUG("Required extensions: ");
    uint32 Length = (uint32)CArrayLength(RequiredExtensions);
    for (uint32 Index = 0; Index < Length; ++Index)
    {
        LUMORA_DEBUG(RequiredExtensions[Index]);
    }
#endif

    CreateInfo.enabledExtensionCount = (uint32)CArrayLength(RequiredExtensions);
    CreateInfo.ppEnabledExtensionNames = RequiredExtensions;

    /** Validation layers. */
    const char** RequiredValidationLayerNames = NULL;
    uint32 RequiredValidationLayerCount = 0;

    /** 
     * If validation should be done, get a list of the required validation layer names
     * and make sure they exist. Validation layers should only be enabled on non-release builds.
     */
#if defined(_DEBUG) || defined(D_DEBUG)
    LUMORA_INFO("Validation layers enabled. Enumerating...");

    /** The list of validation layers required. */
    RequiredValidationLayerNames = CArrayCreate(sizeof(const char*));
    const char* ValidationLayer = "VK_LAYER_KHRONOS_validation";
    CArrayPush(RequiredValidationLayerNames, &ValidationLayer);
    RequiredValidationLayerCount = CArrayLength(RequiredValidationLayerNames);

    /** Obtain a list of available validation layers. */
    uint32 AvailableLayerCount = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&AvailableLayerCount, 0));
    VkLayerProperties* AvailableLayers = CArrayCreateWithCapacity(sizeof(VkLayerProperties), AvailableLayerCount);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&AvailableLayerCount, AvailableLayers));

    /** Verify all required layers are available. */
    for (uint32 Index = 0; Index < RequiredValidationLayerCount; ++Index)
    {
        LUMORA_INFO("Searching for layer: %s...", RequiredValidationLayerNames[Index]);
        bool8 bFound = FALSE;
        for (uint32 InnerIndex = 0; InnerIndex < AvailableLayerCount; ++InnerIndex)
        {
            if (Strcmp(RequiredValidationLayerNames[Index], AvailableLayers[InnerIndex].layerName) == 0)
            {
                bFound = TRUE;
                LUMORA_INFO("Found.");
                break;
            }
        }

        if (!bFound)
        {
            LUMORA_FATAL("Required validation layer is missing: %s", RequiredValidationLayerNames[Index]);
            return FALSE;
        }
    }
#endif

    CreateInfo.ppEnabledLayerNames = RequiredValidationLayerNames;
    CreateInfo.enabledLayerCount = RequiredValidationLayerCount;

    VkResult Result = vkCreateInstance(&CreateInfo, GVulkanContext.Allocator, &GVulkanContext.Instance);
    if (Result != VK_SUCCESS)
    {
        LUMORA_ERROR("VkCreateInstance failed with result: %u", Result);
        return FALSE;
    }
    
    LUMORA_INFO("Vulkan Instance created.");

    /** Create Vulkan debugger. */
#if defined(_DEBUG) || defined(D_DEBUG)
    LUMORA_DEBUG("Creating Vulkan debugger...");
    uint32 LogSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;   //  |
                                                                            //      VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |          
                                                                            //      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
    DebugCreateInfo.messageSeverity = LogSeverity;
    DebugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    DebugCreateInfo.pfnUserCallback = VkDebugCallback;
    DebugCreateInfo.pUserData = NULL;

    PFN_vkCreateDebugUtilsMessengerEXT MessengerFunc = 
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(GVulkanContext.Instance, "vkCreateDebugUtilsMessengerEXT");
    LUMORA_ASSERT_MSG(MessengerFunc, "Failed to create debug messenger.");
    VK_CHECK(MessengerFunc(GVulkanContext.Instance, &DebugCreateInfo, GVulkanContext.Allocator, &GVulkanContext.DebugMessenger));
    LUMORA_DEBUG("Vulkan debugger created.");
#endif
    
    /** Surface */
    LUMORA_DEBUG("Createing Vulkan surface...");
    if (!PlatformCreateVulkanSurface(PlatformState, &GVulkanContext))
    {
        LUMORA_ERROR("Failed to create platform surface.");
        return FALSE;
    }

    LUMORA_DEBUG("Vulkan surface created.");

    /** Device creation */
    if (!VulkanCreateDevice(&GVulkanContext))
    {
        LUMORA_ERROR("Failed to create device.");
        return FALSE;
    }

    /** Swapchain creation. */
    VulkanCreateSwapchain(&GVulkanContext, GVulkanContext.FrameBufferWidth, GVulkanContext.FrameBufferWidth, &GVulkanContext.Swapchain);

    /** RenderPass creation. */
    VulkanCreateRenderPass(&GVulkanContext, &GVulkanContext.MainRenderPass, 0, 0, GVulkanContext.FrameBufferWidth, GVulkanContext.FrameBufferWidth, 0.0f, 0.0f, 0.2f, 1.0f, 1.0f, 0);

    /** Swapchain framebuffers */
    GVulkanContext.Swapchain.FrameBuffers = CArrayCreateWithCapacity(sizeof(FVulkanFrameBuffer), GVulkanContext.Swapchain.ImageCount);
    RegenerateFrameBuffers(Backend, &GVulkanContext.Swapchain, &GVulkanContext.MainRenderPass);

    /** Create command buffers. */
    CreateCommandBuffer(Backend);

    /** Create sync objects. */
    GVulkanContext.ImageAvailableSemaphores = CArrayCreateWithCapacity(sizeof(VkSemaphore), GVulkanContext.Swapchain.MaxFramesInFlight);
    GVulkanContext.QueueCompleteSemaphores  = CArrayCreateWithCapacity(sizeof(VkSemaphore), GVulkanContext.Swapchain.MaxFramesInFlight);
    GVulkanContext.InFlightFences = CArrayCreateWithCapacity(sizeof(FVulkanFence), GVulkanContext.Swapchain.MaxFramesInFlight);

    for (uint8 Index = 0; Index < GVulkanContext.Swapchain.MaxFramesInFlight; ++Index)
    {
        VkSemaphoreCreateInfo SemaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        vkCreateSemaphore(GVulkanContext.Device.Device, &SemaphoreCreateInfo, GVulkanContext.Allocator, &GVulkanContext.ImageAvailableSemaphores[Index]);
        vkCreateSemaphore(GVulkanContext.Device.Device, &SemaphoreCreateInfo, GVulkanContext.Allocator, &GVulkanContext.QueueCompleteSemaphores[Index]);

        /**
         * Create the fence in a signal state, indicating that the first frame has already been "rendered".
         * This will prevent the application from waiting indefinitely for the first frame to render since it
         * cannot be rendered until a frame is "rendered" before it.
         */
        VulkanCreateFence(&GVulkanContext, TRUE, &GVulkanContext.InFlightFences[Index]);
    }

    /** 
     * In flight fences should not yet exist at this point, so clear the list. These are stored in pointers
     * because the initial state should be 0, and will be 0 when not in use. Actual fences are not owned
     * by this list.
     */
    GVulkanContext.ImagesInFlight = CArrayCreateWithCapacity(sizeof(FVulkanFence), GVulkanContext.Swapchain.ImageCount);
    for (uint32 Index = 0; Index < GVulkanContext.Swapchain.ImageCount; ++Index)
    {
        GVulkanContext.ImagesInFlight[Index] = NULL;
    }

    CArrayRelease(RequiredExtensions);

#if defined(_DEBUG) || defined(D_DEBUG)
    CArrayRelease(RequiredValidationLayerNames);
    CArrayRelease(AvailableLayers);
#endif

    LUMORA_INFO("Vulkan renderer initialized successfully.");
    return TRUE;
}

void VulkanReleaseRendererBackend(FRendererBackend* Backend)
{
    vkDeviceWaitIdle(GVulkanContext.Device.Device);

    /** Destroy in the opposite order of creation. */

    /** Sync objects */
    for (uint8 Index = 0; Index < GVulkanContext.Swapchain.MaxFramesInFlight; ++Index)
    {
        if (GVulkanContext.ImageAvailableSemaphores[Index])
        {
            vkDestroySemaphore(GVulkanContext.Device.Device, GVulkanContext.ImageAvailableSemaphores[Index], GVulkanContext.Allocator);
        }
        if (GVulkanContext.QueueCompleteSemaphores[Index])
        {
            vkDestroySemaphore(GVulkanContext.Device.Device, GVulkanContext.QueueCompleteSemaphores[Index], GVulkanContext.Allocator);
        }
        VulkanReleaseFence(&GVulkanContext, &GVulkanContext.InFlightFences[Index]);
    }
    
    CArrayRelease(GVulkanContext.ImageAvailableSemaphores);
    CArrayRelease(GVulkanContext.QueueCompleteSemaphores);
    CArrayRelease(GVulkanContext.InFlightFences);

    GVulkanContext.ImageAvailableSemaphores = NULL;
    GVulkanContext.QueueCompleteSemaphores = NULL;
    GVulkanContext.InFlightFences = NULL;

    /** Command buffers */
    for (uint32 Index = 0; Index < GVulkanContext.Swapchain.ImageCount; ++Index)
    {
        if (GVulkanContext.GraphicsCommandBuffers[Index].Handle)
        {
            VulkanReleaseCommandBuffer(&GVulkanContext, GVulkanContext.Device.GraphicsCommandPool, &GVulkanContext.GraphicsCommandBuffers[Index]);
            GVulkanContext.GraphicsCommandBuffers[Index].Handle = NULL;
            GVulkanContext.GraphicsCommandBuffers[Index].State = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
        }
    }
    CArrayRelease(GVulkanContext.GraphicsCommandBuffers);
    GVulkanContext.GraphicsCommandBuffers = NULL;

    /** Framebuffers */
    for (uint32 Index = 0; Index < GVulkanContext.Swapchain.ImageCount; ++Index)
    {
        VulkanReleaseFrameBuffer(&GVulkanContext, &GVulkanContext.Swapchain.FrameBuffers[Index]);
    }

    /** RenderPass*/
    VulkanReleaseRenderPass(&GVulkanContext, &GVulkanContext.MainRenderPass);

    /** Swapchain */
    VulkanReleaseSwapchain(&GVulkanContext, &GVulkanContext.Swapchain);

    LUMORA_DEBUG("Destroying Vulkan device...");
    VulkanReleaseDevice(&GVulkanContext);

    LUMORA_DEBUG("Destroying Vulkan surface...");
    if (GVulkanContext.Surface)
    {
        vkDestroySurfaceKHR(GVulkanContext.Instance, GVulkanContext.Surface, GVulkanContext.Allocator);
        GVulkanContext.Surface = NULL;
    }

    LUMORA_DEBUG("Release Vulkan debugger...");
    if (GVulkanContext.DebugMessenger)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT MessengerFunc = 
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(GVulkanContext.Instance, "vkDestroyDebugUtilsMessengerEXT");
        MessengerFunc(GVulkanContext.Instance, GVulkanContext.DebugMessenger, GVulkanContext.Allocator);
    }

    LUMORA_DEBUG("Release Vulkan instance...");
    vkDestroyInstance(GVulkanContext.Instance, GVulkanContext.Allocator);
}

void VulkanRendererOnResized(FRendererBackend* Backend, uint16 Width, uint16 Height)
{
}

bool8 VulkanRendererBackendBeginFrame(FRendererBackend* Backend, float32 DeltaTime)
{
    return TRUE;
}

bool8 VulkanRendererBackendEndFrame(FRendererBackend* Backend, float32 DeltaTime)
{
    return TRUE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, 
    VkDebugUtilsMessageTypeFlagsEXT MessageTypes, 
    const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
    void* UserData
) {
    switch (MessageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        LUMORA_ERROR(CallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LUMORA_WARN(CallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        LUMORA_INFO(CallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        LUMORA_TRACE(CallbackData->pMessage);
        break;
    }

    return VK_FALSE;
}

int32 FindMemoryIndex(uint32 TypeFilter, uint32 PropertyFlags)
{
    VkPhysicalDeviceMemoryProperties MemoryProperties = { 0 };
    vkGetPhysicalDeviceMemoryProperties(GVulkanContext.Device.PhysicalDevice, &MemoryProperties);
    
    for (uint32 I = 0; I < MemoryProperties.memoryTypeCount; ++I)
    {
        /** Check each memory type to see if its bit is set to 1. */
        if (TypeFilter & (1 << I) && (MemoryProperties.memoryTypes[I].propertyFlags & PropertyFlags) == PropertyFlags)
        {
            return I;
        }
    }

    LUMORA_WARN("Unable to find suitable memory type.");
    return -1;
}

void CreateCommandBuffer(FRendererBackend* Backend)
{
    LUMORA_UNUSED_PARAM(Backend);

    if (!GVulkanContext.GraphicsCommandBuffers)
    {
        GVulkanContext.GraphicsCommandBuffers = CArrayCreateWithCapacity(sizeof(FVulkanCommandBuffer), GVulkanContext.Swapchain.ImageCount);
        for (uint32 Index = 0; Index < GVulkanContext.Swapchain.ImageCount; ++Index)
        {
            HZeroMemory(&GVulkanContext.GraphicsCommandBuffers[Index], sizeof(FVulkanCommandBuffer));
        }
    }

    for (uint32 Index = 0; Index < GVulkanContext.Swapchain.ImageCount; ++Index)
    {
        if (GVulkanContext.GraphicsCommandBuffers[Index].Handle)
        {
            VulkanReleaseCommandBuffer(&GVulkanContext, GVulkanContext.Device.GraphicsCommandPool, &GVulkanContext.GraphicsCommandBuffers[Index]);
        }

        HZeroMemory(&GVulkanContext.GraphicsCommandBuffers[Index], sizeof(FVulkanCommandBuffer));
        VulkanAllocateCommandBuffer(&GVulkanContext, GVulkanContext.Device.GraphicsCommandPool, TRUE, &GVulkanContext.GraphicsCommandBuffers[Index]);
    }

    LUMORA_INFO("Vulkan command buffer created.");
}

void RegenerateFrameBuffers(FRendererBackend* Backend, FVulkanSwapchain* Swapchain, FVulkanRenderPass* RenderPass)
{
    for (uint32 Index = 0; Index < Swapchain->ImageCount; ++Index)
    {
        /** TODO: make this dynamic based on the currently configured attachments. */
        const uint32 AttachmentCount = 2;
        VkImageView Attachments[] = { Swapchain->ImageViews[Index], Swapchain->DepthAttachment.ImageView };

        VulkanCreateFrameBuffer(
            &GVulkanContext, 
            RenderPass, 
            GVulkanContext.FrameBufferWidth, 
            GVulkanContext.FrameBufferHeight, 
            AttachmentCount, 
            Attachments, 
            &GVulkanContext.Swapchain.FrameBuffers[Index]
        );

    }
}
