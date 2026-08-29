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
#include "Vulkan/VulkanUtils.h"
#include "Vulkan/Shaders/VulkanObjectShader.h"
#include "Vulkan/VulkanBuffer.h"


#include "Core/Math/MathFwd.h"

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

bool8 CreateBuffers(FVulkanContext* VulkanContext);

void CreateCommandBuffer(FRendererBackend* Backend);

void RegenerateFrameBuffers(FRendererBackend* Backend, FVulkanSwapchain* Swapchain, FVulkanRenderPass* RenderPass);

bool8 RecreateSwapchain(FRendererBackend* Backend);

void UploadDataRange(FVulkanContext* VulkanContext, VkCommandPool Pool, VkFence Fence, VkQueue Queue, FVulkanBuffer* Buffer, size_t Offset, size_t Size, void* Data)
{
    /** Create a host-visiable staging buffer to upload to. Mark it as the source of the transfer. */
    VkBufferUsageFlags Flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    FVulkanBuffer Staging = { 0 };
    VulkanCreateBuffer(VulkanContext, Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Flags, TRUE, &Staging);

    /** Load the data into the staging buffer. */
    VulkanBufferLoadData(VulkanContext, &Staging, 0, Size, 0, Data);

    /** Perform the copy from staging to the device local buffer. */
    VulkanBufferCopyTo(VulkanContext, Pool, Fence, Queue, Staging.Handle, 0, Buffer->Handle, Offset, Size);

    /** Clean up the staging buffer. */
    VulkanReleaseBuffer(VulkanContext, &Staging);
}

bool8 VulkanInitializeRendererBackend(FRendererBackend* Backend, const char* ApplicationName)
{
    /** Function pointers */
    GVulkanContext.FindMemoryIndexFunc = FindMemoryIndex;

    /** TODO: Custom Allocator. */
    GVulkanContext.Allocator = NULL;

    ApplicationGetFrameBufferSize(&GCachedFrameBufferWidth, &GCachedFrameBufferHeight);
    GVulkanContext.FrameBufferWidth  = (GCachedFrameBufferWidth  != 0) ? GCachedFrameBufferWidth  : 800;
    GVulkanContext.FrameBufferHeight = (GCachedFrameBufferHeight != 0) ? GCachedFrameBufferHeight : 600;
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
    RequiredValidationLayerCount = (uint32)CArrayLength(RequiredValidationLayerNames);

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
    if (!PlatformCreateVulkanSurface(&GVulkanContext))
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
    VulkanCreateRenderPass(
        &GVulkanContext, 
        &GVulkanContext.MainRenderPass, 
        0, 0, 
        (float32)GVulkanContext.FrameBufferWidth, 
        (float32)GVulkanContext.FrameBufferHeight, 
        1.0f, 1.0f, 1.0f, 1.0f, 
        1.0f, 0.0f
    );

    /** Swapchain framebuffers */
    GVulkanContext.Swapchain.FrameBuffers = CArrayCreateWithCapacity(sizeof(FVulkanFrameBuffer), GVulkanContext.Swapchain.ImageCount);
    RegenerateFrameBuffers(Backend, &GVulkanContext.Swapchain, &GVulkanContext.MainRenderPass);

    /** Create command buffers. */
    CreateCommandBuffer(Backend);

    /** Create sync objects. */
    GVulkanContext.ImageAvailableSemaphores = CArrayCreateWithCapacity(sizeof(VkSemaphore), GVulkanContext.Swapchain.MaxFramesInFlight);
    GVulkanContext.QueueCompleteSemaphores  = CArrayCreateWithCapacity(sizeof(VkSemaphore), GVulkanContext.Swapchain.ImageCount);
    GVulkanContext.InFlightFences = CArrayCreateWithCapacity(sizeof(FVulkanFence), GVulkanContext.Swapchain.MaxFramesInFlight);

    for (uint8 Index = 0; Index < GVulkanContext.Swapchain.MaxFramesInFlight; ++Index)
    {
        VkSemaphoreCreateInfo SemaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        vkCreateSemaphore(GVulkanContext.Device.Device, &SemaphoreCreateInfo, GVulkanContext.Allocator, &GVulkanContext.ImageAvailableSemaphores[Index]);
        //vkCreateSemaphore(GVulkanContext.Device.Device, &SemaphoreCreateInfo, GVulkanContext.Allocator, &GVulkanContext.QueueCompleteSemaphores[Index]);

        /**
         * Create the fence in a signal state, indicating that the first frame has already been "rendered".
         * This will prevent the application from waiting indefinitely for the first frame to render since it
         * cannot be rendered until a frame is "rendered" before it.
         */
        VulkanCreateFence(&GVulkanContext, TRUE, &GVulkanContext.InFlightFences[Index]);
    }

    for (uint32 i = 0; i < GVulkanContext.Swapchain.ImageCount; ++i)
    {
        VkSemaphoreCreateInfo SemaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        vkCreateSemaphore(
            GVulkanContext.Device.Device,
            &SemaphoreCreateInfo,
            GVulkanContext.Allocator,
            &GVulkanContext.QueueCompleteSemaphores[i]);
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

    /** Create builtin shaders */
    if (!VulkanCreateObjectShader(&GVulkanContext, &GVulkanContext.ObjectShader))
    { 
        LUMORA_ERROR("Error loading built-in BasicLighting shader.");
        return FALSE;
    }

    CreateBuffers(&GVulkanContext);

    /** TODO: Tempary test code */
    {
        const uint32 VertexCount = 4;
        FVertex3D Vertices[4] = { 0 };
    
        Vertices[0].Position.X =  0.0f;
        Vertices[0].Position.Y = -0.5f;

        Vertices[1].Position.X =  0.5f;
        Vertices[1].Position.Y =  0.5f;

        Vertices[2].Position.X =  0.0f;
        Vertices[2].Position.Y =  0.5f;

        Vertices[3].Position.X =  0.5f;
        Vertices[3].Position.Y = -0.5f;

        const uint32 IndexCount = 6;
        uint32 Indices[6] = { 
            0, 1, 2, 
            0, 3, 1
        };

        UploadDataRange(
            &GVulkanContext, 
            GVulkanContext.Device.GraphicsCommandPool, 
            NULL, 
            GVulkanContext.Device.GraphicsQueue, 
            &GVulkanContext.ObjectVertexBuffer, 
            0, 
            sizeof(FVertex3D) * VertexCount,
            Vertices
        );

        UploadDataRange(
            &GVulkanContext,
            GVulkanContext.Device.GraphicsCommandPool,
            NULL,
            GVulkanContext.Device.GraphicsQueue,
            &GVulkanContext.ObjectIndexBuffer,
            0,
            sizeof(uint32) * IndexCount,
            Indices
        );
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
    LUMORA_UNUSED_PARAM(Backend);

    vkDeviceWaitIdle(GVulkanContext.Device.Device);

    /** Destroy in the opposite order of creation. */

    /** Destroy buffers */
    VulkanReleaseBuffer(&GVulkanContext, &GVulkanContext.ObjectVertexBuffer);
    VulkanReleaseBuffer(&GVulkanContext, &GVulkanContext.ObjectIndexBuffer);

    VulkanReleaseObjectShader(&GVulkanContext, &GVulkanContext.ObjectShader);

    /** Sync objects */
    for (uint8 Index = 0; Index < GVulkanContext.Swapchain.MaxFramesInFlight; ++Index)
    {
        if (GVulkanContext.ImageAvailableSemaphores[Index])
        {
            vkDestroySemaphore(GVulkanContext.Device.Device, GVulkanContext.ImageAvailableSemaphores[Index], GVulkanContext.Allocator);
        }
        VulkanReleaseFence(&GVulkanContext, &GVulkanContext.InFlightFences[Index]);
    }

    for (uint32 Index = 0; Index < GVulkanContext.Swapchain.ImageCount; ++Index)
    {
        if (GVulkanContext.QueueCompleteSemaphores[Index])
        {
            vkDestroySemaphore(GVulkanContext.Device.Device, GVulkanContext.QueueCompleteSemaphores[Index], GVulkanContext.Allocator);
        }
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
    LUMORA_UNUSED_PARAM(Backend);

    /**
     * Update the "framebuffer size generation", a counter which indicates when the 
     * framebuffer size has been updated.
     */
    GCachedFrameBufferWidth = Width;
    GCachedFrameBufferHeight = Height;
    GVulkanContext.FrameBufferSizeGeneration++;

    LUMORA_INFO("Vulkan renderer backend->resized: Width-Height-Generation: %i-%i-%llu", Width, Height, GVulkanContext.FrameBufferSizeGeneration);
}

bool8 VulkanRendererBackendBeginFrame(FRendererBackend* Backend, float32 DeltaTime)
{
    LUMORA_UNUSED_PARAM(DeltaTime);

    FVulkanDevice* Device = &GVulkanContext.Device;
    
    /** Check if recreating swapchain and boot out. */
    if (GVulkanContext.bRecreatingSwapchain)
    {
        VkResult Result = vkDeviceWaitIdle(Device->Device);
        if (!IsVulkanResultSuccess(Result))
        {
            LUMORA_ERROR("VulkanRendererBackendBeginFrame vkDeviceWaitIdle (1) failed: '%s'", VulkanResultString(Result, TRUE));
            return FALSE;
        }

        LUMORA_INFO("Recreating swapchain, booting.");
        return FALSE;
    }

    /** Check if the framebuffer has been resized. If so, a new swapchain must be created. */
    if (GVulkanContext.FrameBufferSizeGeneration != GVulkanContext.FrameBufferSizeLastGeneration)
    {
        VkResult Result = vkDeviceWaitIdle(Device->Device);
        if (!IsVulkanResultSuccess(Result))
        {
            LUMORA_ERROR("VulkanRendererBackendBeginFrame vkDeviceWaitIdle (1) failed: '%s'", VulkanResultString(Result, TRUE));
            return FALSE;
        }

        /** 
         * If the swapchain recreation failed (because, for example, the window was minimized),
         * boot out before unsetting the flag.
         */
        if (!RecreateSwapchain(Backend))
        {
            return FALSE;
        }

        LUMORA_INFO("Resized, booting.");
        return FALSE;
    }

    /** Wait for the excution fo the current frame to complete. The fence being free will allow this one to move on. */
    if (!VulkanFenceWait(&GVulkanContext, &GVulkanContext.InFlightFences[GVulkanContext.CurrentFrame], UINT64_MAX))
    {
        LUMORA_WARN("Inflight fence wait failure.");
        return FALSE;
    }

    /**
     * Acquire the next image from swapchain. Pass along the semaphore that should signaled when this completes.
     * This same semaphore will later be waited on by the queue submission to ensure this image is available.
     */
    if (!VulkanSwapchainAcquireNextImageIndex(
        &GVulkanContext, 
        &GVulkanContext.Swapchain, 
        UINT64_MAX, 
        GVulkanContext.ImageAvailableSemaphores[GVulkanContext.CurrentFrame], 
        NULL, 
        &GVulkanContext.ImageIndex
    )) {
        return FALSE;
    }

    /** Begin recording commands. */
    FVulkanCommandBuffer* CommandBuffer = &GVulkanContext.GraphicsCommandBuffers[GVulkanContext.ImageIndex];
    VulkanResetCommandBuffer(CommandBuffer);
    VulkanBeginCommandBuffer(CommandBuffer, FALSE, FALSE, FALSE);

    VkViewport ViewPort = { 0 };
    ViewPort.x        =  (float32)0.0f;
    ViewPort.y        =  (float32)GVulkanContext.FrameBufferHeight;
    ViewPort.width    =  (float32)GVulkanContext.FrameBufferWidth;
    ViewPort.height   = -(float32)GVulkanContext.FrameBufferHeight;
    ViewPort.minDepth = 0.0f;
    ViewPort.maxDepth = 1.0f;

    /** Scissor */
    VkRect2D Scissor = { 0 };
    Scissor.extent.width = GVulkanContext.FrameBufferWidth;
    Scissor.extent.height = GVulkanContext.FrameBufferHeight;

    vkCmdSetViewport(CommandBuffer->Handle, 0, 1, &ViewPort);
    vkCmdSetScissor(CommandBuffer->Handle, 0, 1, &Scissor);

    GVulkanContext.MainRenderPass.Width  = (float32)GVulkanContext.FrameBufferWidth;
    GVulkanContext.MainRenderPass.Height = (float32)GVulkanContext.FrameBufferHeight;

    /** Begin the render pass. */
    VulkanRenderPassBegin(CommandBuffer, &GVulkanContext.MainRenderPass, GVulkanContext.Swapchain.FrameBuffers[GVulkanContext.ImageIndex].Handle);

    /** Temporary test code */
    {
        VulkanUseObjectShader(&GVulkanContext, &GVulkanContext.ObjectShader);

        /** Bind vertex buffer at offset */
        VkDeviceSize Offsets[1] = { 0 };
        vkCmdBindVertexBuffers(CommandBuffer->Handle, 0, 1, &GVulkanContext.ObjectVertexBuffer.Handle, (VkDeviceSize*)Offsets);

        /** Bind index buffer at offset. */
        vkCmdBindIndexBuffer(CommandBuffer->Handle, GVulkanContext.ObjectIndexBuffer.Handle, 0, VK_INDEX_TYPE_UINT32);

        /** Issue the draw. */
        vkCmdDrawIndexed(CommandBuffer->Handle, 6, 1, 0, 0, 0);
    }

    return TRUE;
}

bool8 VulkanRendererBackendEndFrame(FRendererBackend* Backend, float32 DeltaTime)
{
    LUMORA_UNUSED_PARAM(Backend);
    LUMORA_UNUSED_PARAM(DeltaTime);

    FVulkanCommandBuffer* CommandBuffer = &GVulkanContext.GraphicsCommandBuffers[GVulkanContext.ImageIndex];

    /** End render pass. */
    VulkanRenderPassEnd(CommandBuffer, &GVulkanContext.MainRenderPass);

    VulkanEndCommandBuffer(CommandBuffer);

    /** Make sure the previous frame is not using this image (i.e. its fencd is being waited on) */
    if (GVulkanContext.ImagesInFlight[GVulkanContext.ImageIndex] != VK_NULL_HANDLE) // Was frame
    {
        VulkanFenceWait(&GVulkanContext, GVulkanContext.ImagesInFlight[GVulkanContext.ImageIndex], UINT64_MAX);
    }

    /** Mark the image fence as in-use by this frame. */
    GVulkanContext.ImagesInFlight[GVulkanContext.ImageIndex] = &GVulkanContext.InFlightFences[GVulkanContext.CurrentFrame];

    /** Reset the fence for use on the next frame. */
    VulkanFenceReset(&GVulkanContext, &GVulkanContext.InFlightFences[GVulkanContext.CurrentFrame]);

    /** 
     * Submit the queue and wait for the operation to complete. 
     * Begin queue submission.
     */
    VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };

    /** Command buffer(s) to be executed. */ 
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffer->Handle;

    /** The semaphore(s) to be signaled thwn the queue is complete. */
    SubmitInfo.signalSemaphoreCount = 1;
    SubmitInfo.pSignalSemaphores = &GVulkanContext.QueueCompleteSemaphores[GVulkanContext.ImageIndex];

    /** Wait semaphore ensures that the operation cannot begin until the image is available. */
    SubmitInfo.waitSemaphoreCount = 1;
    SubmitInfo.pWaitSemaphores = &GVulkanContext.ImageAvailableSemaphores[GVulkanContext.CurrentFrame];

    /**
     * Each semaphore waits on the corresponding pipeline state to complete. 1:1 ratio.
     * VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsquent colour attachment
     * writes from executing until the semaphore signals (i.e. one frame is presented at a time)
     */
    VkPipelineStageFlags Flags[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    SubmitInfo.pWaitDstStageMask = Flags;

    VkResult Result = vkQueueSubmit(GVulkanContext.Device.GraphicsQueue, 1, &SubmitInfo, GVulkanContext.InFlightFences[GVulkanContext.CurrentFrame].Handle);
    if (Result != VK_SUCCESS)
    {
        LUMORA_ERROR("vkQueueSubmit failed with result: %s", VulkanResultString(Result, TRUE));
        return FALSE;
    }

    VulkanUpdateSubmittedCommandBuffer(CommandBuffer);
    /** End queue submission */

    /** Give the image back to the swapchain. */
    VulkanSwapchainPresent(
        &GVulkanContext, 
        &GVulkanContext.Swapchain, 
        GVulkanContext.Device.GraphicsQueue, 
        GVulkanContext.Device.PresentQueue, 
        GVulkanContext.QueueCompleteSemaphores[GVulkanContext.ImageIndex], 
        GVulkanContext.ImageIndex
    );

    return TRUE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, 
    VkDebugUtilsMessageTypeFlagsEXT MessageTypes, 
    const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
    void* UserData
) {
    LUMORA_UNUSED_PARAM(MessageTypes);
    LUMORA_UNUSED_PARAM(UserData);

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

bool8 CreateBuffers(FVulkanContext* VulkanContext)
{
    VkMemoryPropertyFlagBits MemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    const size_t VertexBufferSize = sizeof(FVertex3D) * 1024 * 1024;
    const bool8 bCreateVertexBufferSucceed = VulkanCreateBuffer(
        VulkanContext, 
        VertexBufferSize, 
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        MemoryPropertyFlags, 
        TRUE, 
        &VulkanContext->ObjectVertexBuffer
    );
    if (!bCreateVertexBufferSucceed)
    {
        LUMORA_ERROR("Error creating vertex buffer.");
        return FALSE;
    }

    VulkanContext->GeometryVertexOffset = 0;

    const size_t IndexBufferSize = sizeof(uint32) * 1024 * 1024;
    const bool8 bCreateIndexBufferSucceed = VulkanCreateBuffer(
        VulkanContext,
        IndexBufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        MemoryPropertyFlags,
        TRUE,
        &VulkanContext->ObjectIndexBuffer
    );
    if (!bCreateIndexBufferSucceed)
    {
        LUMORA_ERROR("Error creating index buffer.");
        return FALSE;
    }

    VulkanContext->GeometryIndexOffset = 0;

    return TRUE;
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
    LUMORA_UNUSED_PARAM(Backend);

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

bool8 RecreateSwapchain(FRendererBackend* Backend)
{
    /** If already being recreated, do not try again. */
    if (GVulkanContext.bRecreatingSwapchain)
    {
        LUMORA_DEBUG("RecreateSwapchain called when already recreating. Booting.");
        return FALSE;
    }

    /** Detect if the window is too smail to be drawn to. */
    const bool8 bIsSmallWidth  = GVulkanContext.FrameBufferWidth == 0;
    const bool8 bIsSmallHeight = GVulkanContext.FrameBufferHeight == 0;
    if (bIsSmallWidth || bIsSmallHeight)
    { 
        LUMORA_DEBUG("RecreateSwapchain called when window is < 1 in a dimension. Booting.");
        return FALSE;
    }

    /** Mark as recreating if the demensions are valid. */
    GVulkanContext.bRecreatingSwapchain = TRUE;

    /** Wait for any operations to complete. */
    vkDeviceWaitIdle(GVulkanContext.Device.Device);

    /** Clear these out just in case. */
    for (uint32 Index = 0; Index < GVulkanContext.Swapchain.ImageCount; ++Index)
    {
        GVulkanContext.ImagesInFlight[Index] = NULL;
    }

    /** Requery support */
    VulkanDeviceQuerySwapchainSupport(GVulkanContext.Device.PhysicalDevice, GVulkanContext.Surface, &GVulkanContext.Device.SwapchainSupport);
    VulkanDeviceDetectDepthFormat(&GVulkanContext.Device);

    VulkanRecreateSwapchain(&GVulkanContext, GCachedFrameBufferWidth, GCachedFrameBufferHeight, &GVulkanContext.Swapchain);

    /** Sync the framebuffer size with the cached sizes. */
    GVulkanContext.FrameBufferWidth  = GCachedFrameBufferWidth;
    GVulkanContext.FrameBufferHeight = GCachedFrameBufferHeight;
    GVulkanContext.MainRenderPass.Width  = (float32)GVulkanContext.FrameBufferWidth;
    GVulkanContext.MainRenderPass.Height = (float32)GVulkanContext.FrameBufferHeight;
    GCachedFrameBufferWidth  = 0;
    GCachedFrameBufferHeight = 0;

    /** Update framebuffer size generation. */
    GVulkanContext.FrameBufferSizeLastGeneration = GVulkanContext.FrameBufferSizeGeneration;

    /** Cleanup swapchain */
    for (uint32 Index = 0; Index < GVulkanContext.Swapchain.ImageCount; ++Index)
    {
            (&GVulkanContext, GVulkanContext.Device.GraphicsCommandPool, &GVulkanContext.GraphicsCommandBuffers[Index]);
    }

    /** Freme buffers. */
    for (uint32 Index = 0; Index < GVulkanContext.Swapchain.ImageCount; ++Index)
    {
        VulkanReleaseFrameBuffer(&GVulkanContext, &GVulkanContext.Swapchain.FrameBuffers[Index]);
    }

    GVulkanContext.MainRenderPass.X = 0;
    GVulkanContext.MainRenderPass.Y = 0;
    GVulkanContext.MainRenderPass.Width  = (float32)GVulkanContext.FrameBufferWidth;
    GVulkanContext.MainRenderPass.Height = (float32)GVulkanContext.FrameBufferHeight;

    RegenerateFrameBuffers(Backend, &GVulkanContext.Swapchain, &GVulkanContext.MainRenderPass);

    CreateCommandBuffer(Backend);

    /** Clear the recreating flag. */
    GVulkanContext.bRecreatingSwapchain = FALSE;

    return TRUE;
}
