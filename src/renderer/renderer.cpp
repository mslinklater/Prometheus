#include <iostream>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#include "vulkan/vk_layer_utils.h"

#include "rendererphysicaldevice.h"
#include "rendererlogicaldevice.h"
#include "renderer.h"
#include "rendererutils.h"

#include "system/config.h"
#include "system/log.h"

bool Renderer::sdlInitialised = false;
SDL_WindowFlags Renderer::sdlWindowFlags;
SDL_Window* Renderer::pSdlWindow;

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(	VkDebugReportFlagsEXT flags, 
													VkDebugReportObjectTypeEXT objectType, 
													uint64_t object, 
													size_t location, 
													int32_t messageCode, 
													const char* pLayerPrefix, 
													const char* pMessage, 
													void* pUserData)
{
    (void)flags; 
	(void)object; 
	(void)location; 
	(void)messageCode; 
	(void)pUserData; 
	(void)pLayerPrefix; // Unused arguments
	LOGERRORF("[vulkan] Debug report from ObjectType: %i Message: %s", objectType, pMessage);
    return VK_FALSE;
}

Renderer::Renderer()
{
	vkAllocatorCallbacks = NULL;
	vkInstance = VK_NULL_HANDLE;
	physicalDevice = nullptr;
	device = nullptr;
	vkDevice = VK_NULL_HANDLE;
	vkQueueGraphicsFamily = (uint32_t)-1;
	vkGraphicsQueue = VK_NULL_HANDLE;
	vkDebugReport = VK_NULL_HANDLE;
	vkPipelineCache = VK_NULL_HANDLE;
	vkDescriptorPool = VK_NULL_HANDLE;

	minImageCount = 2;
	swapChainRebuild = false;
	validation = false;
}

Renderer::~Renderer()
{}

void Renderer::Initialise(SDL_Window* window)
{
	sdlWindow = window;

    Setup();

    // Create Window Surface
    if (SDL_Vulkan_CreateSurface(window, vkInstance, &vkSurface) == 0)
    {
        printf("Failed to create Vulkan surface.\n");
        exit(-1);
    }

    // Create Framebuffers
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
	imguiWindow = &imguiVulkanWindowData;
    SetupVulkanWindow(imguiWindow, vkSurface, w, h);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

//    SetupBackends(window);
    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForVulkan(window);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = vkInstance;
    init_info.PhysicalDevice = physicalDevice->GetVkPhysicalDevice();
    init_info.Device = vkDevice;
    init_info.QueueFamily = vkQueueGraphicsFamily;
    init_info.Queue = vkGraphicsQueue;
    init_info.PipelineCache = vkPipelineCache;
    init_info.DescriptorPool = vkDescriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = minImageCount;
    init_info.ImageCount = imguiWindow->ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = vkAllocatorCallbacks;
    init_info.CheckVkResultFn = Renderer::CheckVkResult;
    ImGui_ImplVulkan_Init(&init_info, imguiWindow->RenderPass);

    // Upload Fonts
    {
        // Use any command queue
        VkCommandPool command_pool = imguiWindow->Frames[imguiWindow->FrameIndex].CommandPool;
        VkCommandBuffer command_buffer = imguiWindow->Frames[imguiWindow->FrameIndex].CommandBuffer;

        VkResult err = vkResetCommandPool(vkDevice, command_pool, 0);
        CheckVkResult(err);
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(command_buffer, &begin_info);
        CheckVkResult(err);

        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

        VkSubmitInfo end_info = {};
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &command_buffer;
        err = vkEndCommandBuffer(command_buffer);
        CheckVkResult(err);
        err = vkQueueSubmit(vkGraphicsQueue, 1, &end_info, VK_NULL_HANDLE);
        CheckVkResult(err);

        err = vkDeviceWaitIdle(vkDevice);
        CheckVkResult(err);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
}

void Renderer::Cleanup()
{
    VkResult err = vkDeviceWaitIdle(Renderer::vkDevice);
    CheckVkResult(err);
    ImGui_ImplVulkan_Shutdown();
    CleanupVulkanWindow();
    CleanupVulkan();
}

void Renderer::SetupInstanceLayers()
{
	// Get available instance layers layers
	std::vector<VkLayerProperties> availableInstanceLayers;
    uint32_t numAvailableInstanceLayers;

    vkEnumerateInstanceLayerProperties(&numAvailableInstanceLayers, nullptr);
    availableInstanceLayers.resize(numAvailableInstanceLayers);
    vkEnumerateInstanceLayerProperties(&numAvailableInstanceLayers, &availableInstanceLayers[0]);

	if(Config::GetBool("vulkan.instance.loginfo.layers"))
	{
		LOGINFO("Vulkan::Available instance layers...");
		for(auto layerinfo : availableInstanceLayers)
		{
			LOGINFOF("   %s", layerinfo.layerName);
		}
	}

	if(validation)
	{
		requiredInstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
	}

	// Check that all required layers are available
	for(auto requiredLayer : requiredInstanceLayers)
	{
		bool found = false;
		for(auto availableLayer : availableInstanceLayers)
		{
			if(strcmp(requiredLayer, availableLayer.layerName) == 0)
			{
				found = true;
			}
		}
		if(!found)
		{
			LOGERRORF("Vulkan::Required instance layer not available: %s", requiredLayer);
		}
	}
}

void Renderer::SetupInstanceExtensions()
{
	if(Config::GetBool("vulkan.instance.loginfo.extensions"))
	{
		uint32_t numInstanceExtensions;
		vkEnumerateInstanceExtensionProperties(nullptr, &numInstanceExtensions, nullptr);
		std::vector<VkExtensionProperties> instanceExtensionProperties(numInstanceExtensions);
		vkEnumerateInstanceExtensionProperties(nullptr, &numInstanceExtensions, instanceExtensionProperties.data());
		LOGINFO("Vulkan::Available instance extensions...");
		for(auto extension : instanceExtensionProperties)
		{
			LOGINFOF("   %s(%d)", extension.extensionName, extension.specVersion);
		}
	}

	// Get extensions required by SDL and add them to the required list
    uint32_t sdlExtensionsCount = 0;
    SDL_Vulkan_GetInstanceExtensions(sdlWindow, &sdlExtensionsCount, NULL);
//    const char** sdlExtensions = new const char*[sdlExtensionsCount];
	std::vector<const char*> sdlExtensions(sdlExtensionsCount);
//    SDL_Vulkan_GetInstanceExtensions(sdlWindow, &sdlExtensionsCount, sdlExtensions);
    SDL_Vulkan_GetInstanceExtensions(sdlWindow, &sdlExtensionsCount, sdlExtensions.data());
	for(int iExtension = 0 ; iExtension < sdlExtensionsCount ; ++iExtension)
	{
		requiredInstanceExtensions.push_back(sdlExtensions[iExtension]);
	}
//	delete [] sdlExtensions;


	if(validation)
	{
		requiredInstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	//TODO: Check all required extensions are in fact available
}

void Renderer::SetupDebugReportCallback()
{
    // Get the function pointer (required for any extensions)
    auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(Renderer::vkInstance, "vkCreateDebugReportCallbackEXT");
    assert(vkCreateDebugReportCallbackEXT != NULL);

    // Setup the debug report callback
    VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
    debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    debug_report_ci.pfnCallback = debug_report;
    debug_report_ci.pUserData = NULL;
	VkResult err = vkCreateDebugReportCallbackEXT(vkInstance, &debug_report_ci, vkAllocatorCallbacks, &vkDebugReport);
    CheckVkResult(err);
}

void Renderer::SetupPhysicalDevice()
{
	uint32_t deviceCount;
	VkResult err = vkEnumeratePhysicalDevices(vkInstance, &deviceCount, NULL);
	CheckVkResult(err);
	assert(deviceCount > 0);

	std::vector<VkPhysicalDevice> physicalDeviceHandles(deviceCount); 
	err = vkEnumeratePhysicalDevices(vkInstance, &deviceCount, physicalDeviceHandles.data());
	CheckVkResult(err);

	int selectedDevice = 0;
	for (int i = 0; i < (int)deviceCount; i++)
	{
		// add to physicalDevices vector
		RendererPhysicalDevice* physicalDevice = new RendererPhysicalDevice();
		physicalDevices.push_back(physicalDevice);
		physicalDevice->SetVkPhysicalDevice(physicalDeviceHandles[i]);

		// Issue:#6 Proper choose cirteria
		if (physicalDevice->IsDiscreetGPU())
		{
			selectedDevice = i;
			break;
		}
	}

	physicalDevice = physicalDevices[selectedDevice];
}

void Renderer::SetupQueueFamilies()
{
	vkQueueGraphicsFamily = physicalDevice->GetGraphicsQueueFamilyIndex();
}

void Renderer::SetupLogicalDevice()
{
	int device_extension_count = 1;
	const char* device_extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	const float queue_priority[] = { 1.0f };

	std::vector<VkDeviceQueueCreateInfo> requestedQueueInfo;

	VkDeviceQueueCreateInfo graphicsQueueInfo = {};
	graphicsQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	graphicsQueueInfo.queueFamilyIndex = vkQueueGraphicsFamily;
	graphicsQueueInfo.queueCount = 1;
	graphicsQueueInfo.pQueuePriorities = queue_priority;
	requestedQueueInfo.push_back(graphicsQueueInfo);

	// Issue:#18

	VkDeviceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.queueCreateInfoCount = requestedQueueInfo.size();
	create_info.pQueueCreateInfos = requestedQueueInfo.data();
	create_info.enabledExtensionCount = device_extension_count;
	create_info.ppEnabledExtensionNames = device_extensions;
	VkResult err = vkCreateDevice(physicalDevice->GetVkPhysicalDevice(), &create_info, vkAllocatorCallbacks, &vkDevice);
	CheckVkResult(err);
	
	device = new RendererLogicalDevice(vkDevice);

	vkGetDeviceQueue(vkDevice, vkQueueGraphicsFamily, 0, &vkGraphicsQueue);
}

void Renderer::SetupDescriptorPool()
{
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	VkResult err = vkCreateDescriptorPool(vkDevice, &pool_info, vkAllocatorCallbacks, &vkDescriptorPool);
	CheckVkResult(err);
}

void Renderer::Setup()
{
	validation = Config::GetBool("vulkan.instance.validation");

    VkResult err;

	// Log Vulkan Instance version
	uint32_t instanceVersion;
	err = vkEnumerateInstanceVersion(&instanceVersion);
	CheckVkResult(err);
	LOGINFOF("Vulkan Instance Version %d.%d.%d.%d", VK_API_VERSION_VARIANT(instanceVersion), VK_API_VERSION_MAJOR(instanceVersion), VK_API_VERSION_MINOR(instanceVersion), VK_API_VERSION_PATCH(instanceVersion));

	SetupInstanceLayers();
	SetupInstanceExtensions();

    // Create Vulkan Instance
	VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = NULL;
    applicationInfo.pApplicationName = Config::GetString("application.name").c_str();
    applicationInfo.applicationVersion = 1;
    applicationInfo.pEngineName = "Prometheus";
    applicationInfo.engineVersion = 1;
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledExtensionCount = requiredInstanceExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredInstanceExtensions.data();
    createInfo.enabledLayerCount = requiredInstanceLayers.size();
    createInfo.ppEnabledLayerNames = requiredInstanceLayers.data();

	// Issue#17 - output which extensions & layers are actually requested

	// Create Vulkan Instance
    err = vkCreateInstance(&createInfo, vkAllocatorCallbacks, &vkInstance);
    CheckVkResult(err);

	if(validation)
	{
		SetupDebugReportCallback();
	}

	SetupPhysicalDevice();
	SetupQueueFamilies();
	SetupLogicalDevice();
	SetupDescriptorPool();
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
void Renderer::SetupVulkanWindow(ImGui_ImplVulkanH_Window* _imguiWindow, VkSurfaceKHR surface, int width, int height)
{
	imguiWindow = _imguiWindow;
    imguiWindow->Surface = surface;

    // Check for WSI support
    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice->GetVkPhysicalDevice(), vkQueueGraphicsFamily, imguiWindow->Surface, &res);
    if (res != VK_TRUE)
    {
        fprintf(stderr, "Error no WSI support on physical device 0\n");
        exit(-1);
    }

	std::vector<VkFormat> requestSurfaceImageFormats = {
		VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM
	};

	imguiWindow->SurfaceFormat = RendererUtils::FindBestSurfaceFormat(
		physicalDevice->GetVkPhysicalDevice(),
		requestSurfaceImageFormats, 
		VK_COLORSPACE_SRGB_NONLINEAR_KHR,
		imguiWindow->Surface);

    // Select Present Mode
#if 0
	// go as fast as you can
	std::vector<VkPresentModeKHR> presentModes = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
	std::vector<VkPresentModeKHR> presentModes = { VK_PRESENT_MODE_FIFO_KHR };
#endif

    imguiWindow->PresentMode = RendererUtils::FindBestPresentMode(physicalDevice->GetVkPhysicalDevice(), imguiWindow->Surface, presentModes);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    assert(minImageCount >= 2);

	// TODO: break this up too...
    ImGui_ImplVulkanH_CreateOrResizeWindow(vkInstance, physicalDevice->GetVkPhysicalDevice(), vkDevice, imguiWindow, vkQueueGraphicsFamily, vkAllocatorCallbacks, width, height, minImageCount);
}

void Renderer::CleanupVulkan()
{
    vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, vkAllocatorCallbacks);

    // Remove the debug report callback
    auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
    vkDestroyDebugReportCallbackEXT(vkInstance, vkDebugReport, vkAllocatorCallbacks);

    vkDestroyDevice(vkDevice, vkAllocatorCallbacks);
    vkDestroyInstance(vkInstance, vkAllocatorCallbacks);
}

void Renderer::CleanupVulkanWindow()
{
    ImGui_ImplVulkanH_DestroyWindow(vkInstance, vkDevice, &imguiVulkanWindowData, vkAllocatorCallbacks);
}

void Renderer::BeginFrame()
{
	// Resize swap chain?
	if (swapChainRebuild)
	{
		int width, height;
		SDL_GetWindowSize(sdlWindow, &width, &height);
		if (width > 0 && height > 0)
		{
			ImGui_ImplVulkan_SetMinImageCount(minImageCount);
			ImGui_ImplVulkanH_CreateOrResizeWindow(vkInstance, physicalDevice->GetVkPhysicalDevice(), vkDevice, &imguiVulkanWindowData, vkQueueGraphicsFamily, vkAllocatorCallbacks, width, height, minImageCount);
			imguiVulkanWindowData.FrameIndex = 0;
			swapChainRebuild = false;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame();
}


void Renderer::FrameRender(ImDrawData* draw_data)
{
    VkResult err;

    VkSemaphore image_acquired_semaphore  = imguiWindow->FrameSemaphores[imguiWindow->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = imguiWindow->FrameSemaphores[imguiWindow->SemaphoreIndex].RenderCompleteSemaphore;
    err = vkAcquireNextImageKHR(vkDevice, imguiWindow->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &imguiWindow->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        swapChainRebuild = true;
        return;
    }
    CheckVkResult(err);

    ImGui_ImplVulkanH_Frame* fd = &imguiWindow->Frames[imguiWindow->FrameIndex];
    {
        err = vkWaitForFences(vkDevice, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
        CheckVkResult(err);

        err = vkResetFences(vkDevice, 1, &fd->Fence);
        CheckVkResult(err);
    }
    {
        err = vkResetCommandPool(vkDevice, fd->CommandPool, 0);
        CheckVkResult(err);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        CheckVkResult(err);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = imguiWindow->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = imguiWindow->Width;
        info.renderArea.extent.height = imguiWindow->Height;
        info.clearValueCount = 1;
        info.pClearValues = &imguiWindow->ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        CheckVkResult(err);
        err = vkQueueSubmit(vkGraphicsQueue, 1, &info, fd->Fence);
        CheckVkResult(err);
    }
}

void Renderer::FramePresent()
{
    if (swapChainRebuild)
        return;
    VkSemaphore render_complete_semaphore = imguiWindow->FrameSemaphores[imguiWindow->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &imguiWindow->Swapchain;
    info.pImageIndices = &imguiWindow->FrameIndex;
    VkResult err = vkQueuePresentKHR(vkGraphicsQueue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        swapChainRebuild = true;
        return;
    }
    CheckVkResult(err);
    imguiWindow->SemaphoreIndex = (imguiWindow->SemaphoreIndex + 1) % imguiWindow->ImageCount; // Now we can use the next set of semaphores
}

#if 0
EError Renderer::Init()
{
    LOGVERBOSE("Renderer:Init()");
    assert(sdlInitialised);

	validation = Config::Instance()->GetBool("vulkan.instance.validation");

    // Get available instance layers

    uint32_t numAvailableLayers;
    vkEnumerateInstanceLayerProperties(&numAvailableLayers, nullptr);
    availableLayers.resize(numAvailableLayers);
    vkEnumerateInstanceLayerProperties(&numAvailableLayers, &availableLayers[0]);

	// Check we have the validation layer available and add it to the enabled layers array
	if(validation)
	{
		EnableValidation();
	}

    // Get available instance extensions
    uint32_t numAvailableExtensions;
    vkEnumerateInstanceExtensionProperties(nullptr, &numAvailableExtensions, nullptr);
    availableExtensions.resize(numAvailableExtensions);
    vkEnumerateInstanceExtensionProperties(nullptr, &numAvailableExtensions, &availableExtensions[0]);

	GetRequiredAndOptionalExtensions();
	CheckRequiredExtensions();

    // Create instance

    // VkApplicationInfo allows the programmer to specifiy some basic
    // information about the program, which can be useful for layers and tools
    // to provide more debug information.
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = NULL;
    applicationInfo.pApplicationName = "Prometheus";
    applicationInfo.applicationVersion = 1;
    applicationInfo.pEngineName = "Prometheus";
    applicationInfo.engineVersion = 1;
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    // VkInstanceCreateInfo is where the programmer specifies the layers and/or
    // extensions that are needed.
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = NULL;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &applicationInfo;
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(requestedExtensions.size());
    instanceInfo.ppEnabledExtensionNames = requestedExtensions.data();
    instanceInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
    instanceInfo.ppEnabledLayerNames = enabledLayers.data();

//	Check required extensions are supported...

    // Create the Vulkan instance.
    VkResult result = vkCreateInstance(&instanceInfo, NULL, &instance);
    if (result == VK_ERROR_INCOMPATIBLE_DRIVER)
    {
        return ErrorLogAndReturn(EError::Vulkan_UnableToFindDriver);
    }
    else if (result == VK_ERROR_EXTENSION_NOT_PRESENT)
    {
        return ErrorLogAndReturn(EError::Vulkan_ExtensionNotPresent);
    }
    else if (result)
    {
        return ErrorLogAndReturn(EError::Vulkan_CouldNotCreateInstance);
    }
	LOGINFO("Vulkan::Instance created");
    
    // Enumerate physical devices

    uint32_t physicalDeviceCount;
    result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    if (result)
    {
        return ErrorLogAndReturn(EError::Vulkan_CouldNotEnumeratePhysicalDevices);
    }

    physicalDevices.resize(physicalDeviceCount);

    std::vector<VkPhysicalDevice> vulkanPhysicalDevices(physicalDeviceCount);

    result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, &vulkanPhysicalDevices[0]);
    if (result)
    {
        return ErrorLogAndReturn(EError::Vulkan_CouldNotEnumeratePhysicalDevices);
    }

    for (uint32_t iDevice = 0; iDevice < physicalDeviceCount; iDevice++)
    {
        physicalDevices[iDevice].SetPhysicalDevice(vulkanPhysicalDevices[iDevice]);
    }

    if(Config::Instance()->GetBool("vulkan.instance.loginfo"))
    {
        LogInstanceProperties();
    }
    if(Config::Instance()->GetBool("vulkan.devices.loginfo"))
    {
        for (int iDevice = 0; iDevice < physicalDevices.size(); iDevice++)
        {
            physicalDevices[iDevice].LogDeviceInfo();
        }
    }

    // TODO: choose which physical device to use and create the logical device

	// go through acceptablePhysicalDevices vector and test against desired properties, removing ones which don't satisfy requirements
    for (uint32_t iDevice = 0; iDevice < physicalDeviceCount; iDevice++)
	{
		if((Config::Instance()->GetBool("vulkan.require.queue.graphics")) && (physicalDevices[iDevice].GraphicsQueueIndex() == -1))
        {
            physicalDevices[iDevice].acceptable = false;
        }
    }

	// knock out ignored devices
	std::vector<std::string> ignoredDevices = Config::Instance()->GetStringVector("vulkan.device.ignored");
	for(auto ignore : ignoredDevices)
	{
		for( RendererPhysicalDevice& physicalDevice : physicalDevices)
		{
			if(physicalDevice.GetName().find(ignore) != std::string::npos)
			{
				physicalDevice.acceptable = false;
			}
		}
	}

    // check for a preferred vulkan device
    if (Config::Instance()->StringVectorExists("vulkan.device.preferred"))
    {
        // find the preferred device
        std::vector<std::string> preferredDevices = Config::Instance()->GetStringVector("vulkan.device.preferred");
    }
    else
    {
        // find the best device with required caps
    }

    // Create a Vulkan surface for rendering
    if (!SDL_Vulkan_CreateSurface(pSdlWindow, instance, &surface))
    {
        return ErrorLogAndReturn(EError::SDL_CouldNotCreateVulkanSurface);
    }

	// Create the first acceptable logical device

	pLogicalDevice = nullptr;

	for(RendererPhysicalDevice& physicalDevice : physicalDevices )
	{
		if(physicalDevice.acceptable)
		{
			pLogicalDevice = std::make_shared<RendererLogicalDevice>(physicalDevices[0].GetPhysicalDevice());
			break;
		}
	}

	if(pLogicalDevice == nullptr)
	{
		LOGFATAL("Vulkan::cannot find suitable physical device");
	}

    return EError::OK;
}
#endif

void Renderer::CheckVkResult(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}
