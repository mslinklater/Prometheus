#include <iostream>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#include "vulkaninstance.h"
#include "vulkanphysicaldevice.h"
#include "vulkanlogicaldevice.h"

#include "renderer.h"
#include "rendererutils.h"

#include "system/config.h"
#include "system/log.h"

#include "backends/imgui_impl_vulkan.h"

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
: instance(nullptr)
{
	vkAllocatorCallbacks = NULL;

	physicalDevice = nullptr;	// TODO: smart pointer ?
	device = nullptr;	// TODO: smart pointer ?

	queueGraphicsFamilyIndex = (uint32_t)-1;
	graphicsQueue = VK_NULL_HANDLE;
	debugReportExtension = VK_NULL_HANDLE;
	pipelineCache = VK_NULL_HANDLE;
	descriptorPool = VK_NULL_HANDLE;

	minImageCount = 2;
	swapChainRebuild = false;
	validation = false;

	swapchain = VK_NULL_HANDLE;
	renderPass = VK_NULL_HANDLE;
	pipeline = VK_NULL_HANDLE;
	semaphoreIndex = 0;
}

Renderer::~Renderer()
{}

void Renderer::DrawVulkanDebugWindow()
{
	static bool open = true;

	if(!ImGui::Begin("Vulkan Debug", &open, ImGuiWindowFlags_MenuBar))
	{
		// collapsed, so early out
		ImGui::End();
	}
	else
	{
		static bool dumpToLog;

		// menu bar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Tools"))
			{
				ImGui::MenuItem("Dump to log", NULL, &dumpToLog);
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		// Instance
		if (ImGui::CollapsingHeader("Instance"))
		{
			instance->DrawDebug();
		}
		// Physical Devices
		if (ImGui::CollapsingHeader("Physical Devices"))
		{
			for(auto physicalDevice : physicalDevices)
			{
				physicalDevice->DrawDebug();
			}
		}
		if (ImGui::CollapsingHeader("Logical Device"))
		{
//			instance->DrawDebug();
		}

		ImGui::End();
	}
}

void Renderer::Initialise(SDL_Window* _window)
{
	sdlWindow = _window;

    Setup();

    // Create Window Surface
    if (SDL_Vulkan_CreateSurface(sdlWindow, instance->GetVkInstance(), &window.surface) == 0)
    {
        printf("Failed to create Vulkan surface.\n");
        exit(-1);
    }

    // Create Framebuffers
    int w, h;
    SDL_GetWindowSize(sdlWindow, &w, &h);

    SetupVulkanWindow(w, h);

	ImGuiSetup();

}

void Renderer::ImGuiSetup()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForVulkan(sdlWindow);
    ImGui_ImplVulkan_InitInfo init_info = {};

    init_info.Instance = instance->GetVkInstance();
    init_info.PhysicalDevice = physicalDevice->GetVkPhysicalDevice();
    init_info.Device = device->GetVkDevice();
    init_info.QueueFamily = queueGraphicsFamilyIndex;
    init_info.Queue = graphicsQueue;
    init_info.PipelineCache = pipelineCache;
    init_info.DescriptorPool = descriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = minImageCount;
    init_info.ImageCount = window.imageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = vkAllocatorCallbacks;
    init_info.CheckVkResultFn = CheckVkResult;
    ImGui_ImplVulkan_Init(&init_info, renderPass);

    // Upload Fonts
    {
        // Use any command queue
        VkCommandPool command_pool = frames[window.frameIndex].CommandPool;
        VkCommandBuffer command_buffer = frames[window.frameIndex].CommandBuffer;

        VkResult err = vkResetCommandPool(device->GetVkDevice(), command_pool, 0);
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
        err = vkQueueSubmit(graphicsQueue, 1, &end_info, VK_NULL_HANDLE);
        CheckVkResult(err);

        err = vkDeviceWaitIdle(device->GetVkDevice());
        CheckVkResult(err);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
}

void Renderer::Cleanup()
{
    VkResult err = vkDeviceWaitIdle(device->GetVkDevice());
    CheckVkResult(err);
    ImGui_ImplVulkan_Shutdown();
    CleanupVulkanWindow();
    CleanupVulkan();
}

void Renderer::SetupDebugReportCallback()
{
    auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance->GetVkInstance(), "vkCreateDebugReportCallbackEXT");
    assert(vkCreateDebugReportCallbackEXT != NULL);

    // Setup the debug report callback
    VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
    debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    debug_report_ci.pfnCallback = debug_report;
    debug_report_ci.pUserData = NULL;
	VkResult err = vkCreateDebugReportCallbackEXT(instance->GetVkInstance(), &debug_report_ci, vkAllocatorCallbacks, &debugReportExtension);
    CheckVkResult(err);
}

void Renderer::SetupPhysicalDevice()
{
	uint32_t deviceCount;
	VkResult err = vkEnumeratePhysicalDevices(instance->GetVkInstance(), &deviceCount, NULL);
	CheckVkResult(err);
	assert(deviceCount > 0);

	std::vector<VkPhysicalDevice> physicalDeviceHandles(deviceCount); 
	err = vkEnumeratePhysicalDevices(instance->GetVkInstance(), &deviceCount, physicalDeviceHandles.data());
	CheckVkResult(err);

	int selectedDevice = 0;
	for (int i = 0; i < (int)deviceCount; i++)
	{
		// add to physicalDevices vector
		VulkanPhysicalDevice* physicalDevice = new VulkanPhysicalDevice();
		physicalDevices.push_back(physicalDevice);
		physicalDevice->SetVkPhysicalDevice(physicalDeviceHandles[i]);
	}

	for (int i = 0; i < (int)deviceCount; i++)
	{
		// Issue:#6 Proper choose cirteria
		if (physicalDevices[i]->IsDiscreetGPU())
		{
			selectedDevice = i;
			break;
		}
	}

	physicalDevice = physicalDevices[selectedDevice];
}

void Renderer::SetupQueueFamilies()
{
	queueGraphicsFamilyIndex = physicalDevice->GetGraphicsQueueFamilyIndex();
}

void Renderer::SetupLogicalDevice()
{
	int device_extension_count = 1;
	const char* device_extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	const float queue_priority[] = { 1.0f };

	std::vector<VkDeviceQueueCreateInfo> requestedQueueInfo;

	VkDeviceQueueCreateInfo graphicsQueueInfo = {};
	graphicsQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	graphicsQueueInfo.queueFamilyIndex = queueGraphicsFamilyIndex;
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
	VkDevice vkDevice;
	VkResult err = vkCreateDevice(physicalDevice->GetVkPhysicalDevice(), &create_info, vkAllocatorCallbacks, &vkDevice);
	CheckVkResult(err);
	
	device = new VulkanLogicalDevice(vkDevice);

	vkGetDeviceQueue(device->GetVkDevice(), queueGraphicsFamilyIndex, 0, &graphicsQueue);
}

void Renderer::SetClearValue(float r, float g, float b, float a)
{
	clearValue.color.float32[0] = r;
	clearValue.color.float32[1] = g;
	clearValue.color.float32[2] = b;
	clearValue.color.float32[3] = a;
}

void Renderer::SetupDescriptorPool()
{
	const static uint32_t POOL_SIZE = 1000;

	std::vector<VkDescriptorPoolSize> pool_sizes =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, POOL_SIZE }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.poolSizeCount = pool_sizes.size();
	pool_info.maxSets = POOL_SIZE * pool_info.poolSizeCount;
	pool_info.pPoolSizes = pool_sizes.data();
	VkResult err = vkCreateDescriptorPool(device->GetVkDevice(), &pool_info, vkAllocatorCallbacks, &descriptorPool);
	CheckVkResult(err);
}

void Renderer::Setup()
{
	validation = Config::GetBool("vulkan.instance.validation");

	instance = new VulkanInstance(vkAllocatorCallbacks, validation, sdlWindow);

	if(validation)
	{
		SetupDebugReportCallback();
	}

	SetupPhysicalDevice();
	SetupQueueFamilies();
	SetupLogicalDevice();
	SetupDescriptorPool();
}

void Renderer::SetupVulkanWindow(int width, int height)
{
    // Check for WSI support
    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice->GetVkPhysicalDevice(), queueGraphicsFamilyIndex, window.surface, &res);
    if (res != VK_TRUE)
    {
        fprintf(stderr, "Error no WSI support on physical device 0\n");
        exit(-1);
    }

	std::vector<VkFormat> requestSurfaceImageFormats = {
		VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM
	};

	window.surfaceFormat = RendererUtils::FindBestSurfaceFormat(
		physicalDevice->GetVkPhysicalDevice(),
		requestSurfaceImageFormats, 
		VK_COLORSPACE_SRGB_NONLINEAR_KHR,
		window.surface);

    // Select Present Mode
#if 0
	// go as fast as you can
	std::vector<VkPresentModeKHR> presentModes = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
	std::vector<VkPresentModeKHR> presentModes = { VK_PRESENT_MODE_FIFO_KHR };
#endif

    window.presentMode = RendererUtils::FindBestPresentMode(physicalDevice->GetVkPhysicalDevice(), window.surface, presentModes);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    assert(minImageCount >= 2);

	CreateOrResizeWindow(width, height);
}

void Renderer::CreateOrResizeWindow(uint32_t width, uint32_t height)
{
	VkResult err;

	VkSwapchainKHR old_swapchain = swapchain;
	swapchain = VK_NULL_HANDLE;

	err = vkDeviceWaitIdle(device->GetVkDevice());
	CheckVkResult(err);

	// Destroy existing swapchain images and associated buffers
	for (uint32_t iImage = 0; iImage < window.imageCount; iImage++)
	{
		FrameData* fd = &frames[iImage];
		vkDestroyFence(device->GetVkDevice(), fd->Fence, vkAllocatorCallbacks);
		vkFreeCommandBuffers(device->GetVkDevice(), fd->CommandPool, 1, &fd->CommandBuffer);
		vkDestroyCommandPool(device->GetVkDevice(), fd->CommandPool, vkAllocatorCallbacks);
		fd->Fence = VK_NULL_HANDLE;
		fd->CommandBuffer = VK_NULL_HANDLE;
		fd->CommandPool = VK_NULL_HANDLE;

		vkDestroyImageView(device->GetVkDevice(), fd->BackbufferView, vkAllocatorCallbacks);
		vkDestroyFramebuffer(device->GetVkDevice(), fd->Framebuffer, vkAllocatorCallbacks);

		FrameSemaphores* fsd = &frameSemaphores[iImage];
		vkDestroySemaphore(device->GetVkDevice(), fsd->ImageAcquiredSemaphore, vkAllocatorCallbacks);
		vkDestroySemaphore(device->GetVkDevice(), fsd->RenderCompleteSemaphore, vkAllocatorCallbacks);
		fsd->ImageAcquiredSemaphore = fsd->RenderCompleteSemaphore = VK_NULL_HANDLE;
	}

	frames.clear();
	frameSemaphores.clear();

	window.imageCount = 0;

	if (renderPass)
		vkDestroyRenderPass(device->GetVkDevice(), renderPass, vkAllocatorCallbacks);
	if (pipeline)
		vkDestroyPipeline(device->GetVkDevice(), pipeline, vkAllocatorCallbacks);

	// If min image count was not specified, request different count of images dependent on selected present mode
	if (minImageCount == 0)
	{
		if (window.presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			minImageCount = 3;
		if (window.presentMode == VK_PRESENT_MODE_FIFO_KHR || window.presentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
			minImageCount = 2;
		if (window.presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			minImageCount = 1;
	}

	// Create new Swapchain
	{
		VkSwapchainCreateInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.surface = window.surface;
		info.minImageCount = minImageCount;
		info.imageFormat = window.surfaceFormat.format;
		info.imageColorSpace = window.surfaceFormat.colorSpace;
		info.imageArrayLayers = 1;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;           // Assume that graphics family == present family
		info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.presentMode = window.presentMode;
		info.clipped = VK_TRUE;
		info.oldSwapchain = old_swapchain;
		VkSurfaceCapabilitiesKHR cap;

		err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice->GetVkPhysicalDevice(), window.surface, &cap);
		CheckVkResult(err);

		if (info.minImageCount < cap.minImageCount)
			info.minImageCount = cap.minImageCount;
		else if (cap.maxImageCount != 0 && info.minImageCount > cap.maxImageCount)
			info.minImageCount = cap.maxImageCount;

		if (cap.currentExtent.width == 0xffffffff)
		{
			info.imageExtent.width = window.width = width;
			info.imageExtent.height = window.height = height;
		}
		else
		{
			info.imageExtent.width = window.width = cap.currentExtent.width;
			info.imageExtent.height = window.height = cap.currentExtent.height;
		}
		err = vkCreateSwapchainKHR(device->GetVkDevice(), &info, vkAllocatorCallbacks, &swapchain);
		CheckVkResult(err);
		err = vkGetSwapchainImagesKHR(device->GetVkDevice(), swapchain, &window.imageCount, nullptr);
		CheckVkResult(err);

		static const uint32_t BACK_BUFFER_SIZE = 16;

		VkImage backbuffers[BACK_BUFFER_SIZE] = {};
		assert(window.imageCount >= minImageCount);
		assert(window.imageCount < BACK_BUFFER_SIZE);
		err = vkGetSwapchainImagesKHR(device->GetVkDevice(), swapchain, &window.imageCount, backbuffers);
		CheckVkResult(err);

		assert(frames.size() == 0);

		// TODO: change to std::vector
		frames.resize(window.imageCount);
		frameSemaphores.resize(window.imageCount);
		memset(frames.data(), 0, sizeof(frames[0]) * window.imageCount);
		memset(frameSemaphores.data(), 0, sizeof(frameSemaphores[0]) * window.imageCount);

		for (uint32_t i = 0; i < window.imageCount; i++)
			frames[i].Backbuffer = backbuffers[i];
	}
	if (old_swapchain)
		vkDestroySwapchainKHR(device->GetVkDevice(), old_swapchain, vkAllocatorCallbacks);

	// Create the Render Pass
	{
		VkAttachmentDescription attachment = {};
		attachment.format = window.surfaceFormat.format;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = window.clearEnable ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		VkAttachmentReference color_attachment = {};
		color_attachment.attachment = 0;
		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment;
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1;
		info.pAttachments = &attachment;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 1;
		info.pDependencies = &dependency;
		err = vkCreateRenderPass(device->GetVkDevice(), &info, vkAllocatorCallbacks, &renderPass);
		CheckVkResult(err);
	}

	// Create The Image Views
	{
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = window.surfaceFormat.format;
		info.components.r = VK_COMPONENT_SWIZZLE_R;
		info.components.g = VK_COMPONENT_SWIZZLE_G;
		info.components.b = VK_COMPONENT_SWIZZLE_B;
		info.components.a = VK_COMPONENT_SWIZZLE_A;
		VkImageSubresourceRange image_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		info.subresourceRange = image_range;

		for (uint32_t i = 0; i < window.imageCount; i++)
		{
			FrameData* fd = &frames[i];
			info.image = fd->Backbuffer;
			err = vkCreateImageView(device->GetVkDevice(), &info, vkAllocatorCallbacks, &fd->BackbufferView);
			CheckVkResult(err);
		}
	}

	// Create Framebuffer
	{
		VkImageView attachment[1];
		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.renderPass = renderPass;
		info.attachmentCount = 1;
		info.pAttachments = attachment;
		info.width = window.width;
		info.height = window.height;
		info.layers = 1;

		for (uint32_t i = 0; i < window.imageCount; i++)
		{
			FrameData* fd = &frames[i];
			attachment[0] = fd->BackbufferView;
			err = vkCreateFramebuffer(device->GetVkDevice(), &info, vkAllocatorCallbacks, &fd->Framebuffer);
			CheckVkResult(err);
		}
	}

	// Create Command Buffers
	for (uint32_t i = 0; i < window.imageCount; i++)
	{
		FrameData* fd = &frames[i];
		FrameSemaphores* fsd = &frameSemaphores[i];
		{
			VkCommandPoolCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			info.queueFamilyIndex = queueGraphicsFamilyIndex;
			err = vkCreateCommandPool(device->GetVkDevice(), &info, vkAllocatorCallbacks, &fd->CommandPool);
			CheckVkResult(err);
		}
		{
			VkCommandBufferAllocateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			info.commandPool = fd->CommandPool;
			info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			info.commandBufferCount = 1;
			err = vkAllocateCommandBuffers(device->GetVkDevice(), &info, &fd->CommandBuffer);
			CheckVkResult(err);
		}
		{
			VkFenceCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			err = vkCreateFence(device->GetVkDevice(), &info, vkAllocatorCallbacks, &fd->Fence);
			CheckVkResult(err);
		}
		{
			VkSemaphoreCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			err = vkCreateSemaphore(device->GetVkDevice(), &info, vkAllocatorCallbacks, &fsd->ImageAcquiredSemaphore);
			CheckVkResult(err);
			err = vkCreateSemaphore(device->GetVkDevice(), &info, vkAllocatorCallbacks, &fsd->RenderCompleteSemaphore);
			CheckVkResult(err);
		}
	}
}

void Renderer::CleanupVulkan()
{
    vkDestroyDescriptorPool(device->GetVkDevice(), descriptorPool, vkAllocatorCallbacks);

    // Remove the debug report callback
//    auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
    auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance->GetVkInstance(), "vkDestroyDebugReportCallbackEXT");
//    vkDestroyDebugReportCallbackEXT(vkInstance, vkDebugReport, vkAllocatorCallbacks);
    vkDestroyDebugReportCallbackEXT(instance->GetVkInstance(), debugReportExtension, vkAllocatorCallbacks);

    vkDestroyDevice(device->GetVkDevice(), vkAllocatorCallbacks);
//    vkDestroyInstance(vkInstance, vkAllocatorCallbacks);
    vkDestroyInstance(instance->GetVkInstance(), vkAllocatorCallbacks);
}

void Renderer::CleanupVulkanWindow()
{
    vkDeviceWaitIdle(device->GetVkDevice());

    for (uint32_t i = 0; i < window.imageCount; i++)
    {
		FrameData* fd = &frames[i];
		{
			vkDestroyFence(device->GetVkDevice(), fd->Fence, vkAllocatorCallbacks);
			vkFreeCommandBuffers(device->GetVkDevice(), fd->CommandPool, 1, &fd->CommandBuffer);
			vkDestroyCommandPool(device->GetVkDevice(), fd->CommandPool, vkAllocatorCallbacks);
			fd->Fence = VK_NULL_HANDLE;
			fd->CommandBuffer = VK_NULL_HANDLE;
			fd->CommandPool = VK_NULL_HANDLE;

			vkDestroyImageView(device->GetVkDevice(), fd->BackbufferView, vkAllocatorCallbacks);
			vkDestroyFramebuffer(device->GetVkDevice(), fd->Framebuffer, vkAllocatorCallbacks);
		}

		FrameSemaphores* fsd = &frameSemaphores[i];
		{
			vkDestroySemaphore(device->GetVkDevice(), fsd->ImageAcquiredSemaphore, vkAllocatorCallbacks);
			vkDestroySemaphore(device->GetVkDevice(), fsd->RenderCompleteSemaphore, vkAllocatorCallbacks);
			fsd->ImageAcquiredSemaphore = fsd->RenderCompleteSemaphore = VK_NULL_HANDLE;
		}
    }
    frames.clear();
    frameSemaphores.clear();
    vkDestroyPipeline(device->GetVkDevice(), pipeline, vkAllocatorCallbacks);
    vkDestroyRenderPass(device->GetVkDevice(), renderPass, vkAllocatorCallbacks);
    vkDestroySwapchainKHR(device->GetVkDevice(), swapchain, vkAllocatorCallbacks);
    vkDestroySurfaceKHR(instance->GetVkInstance(), window.surface, vkAllocatorCallbacks);
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

			Renderer::CreateOrResizeWindow(width, height);

			window.frameIndex = 0;
			swapChainRebuild = false;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame();
}


void Renderer::ImGuiRender(ImDrawData* draw_data)
{
    VkResult err;

    VkSemaphore image_acquired_semaphore  = frameSemaphores[semaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = frameSemaphores[semaphoreIndex].RenderCompleteSemaphore;
	
    err = vkAcquireNextImageKHR(device->GetVkDevice(), swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &window.frameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        swapChainRebuild = true;
        return;
    }
    CheckVkResult(err);

    FrameData* fd = &frames[window.frameIndex];
    {
        err = vkWaitForFences(device->GetVkDevice(), 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
        CheckVkResult(err);

        err = vkResetFences(device->GetVkDevice(), 1, &fd->Fence);
        CheckVkResult(err);
    }
    {
        err = vkResetCommandPool(device->GetVkDevice(), fd->CommandPool, 0);
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
        info.renderPass = renderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = window.width;
        info.renderArea.extent.height = window.height;
        info.clearValueCount = 1;
        info.pClearValues = &clearValue;
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
        err = vkQueueSubmit(graphicsQueue, 1, &info, fd->Fence);
        CheckVkResult(err);
    }
}

void Renderer::FramePresent()
{
    if (swapChainRebuild)
        return;
    VkSemaphore render_complete_semaphore = frameSemaphores[semaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &swapchain;
    info.pImageIndices = &window.frameIndex;
    VkResult err = vkQueuePresentKHR(graphicsQueue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        swapChainRebuild = true;
        return;
    }
    CheckVkResult(err);
    semaphoreIndex = (semaphoreIndex + 1) % window.imageCount; // Now we can use the next set of semaphores
}

