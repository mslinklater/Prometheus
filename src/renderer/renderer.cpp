#include <iostream>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#include "vulkan/vk_layer_utils.h"

#include "rendererlogicaldevice.h"
#include "renderer.h"
#include "system/config.h"
#include "system/log.h"

// moved over
VkAllocationCallbacks*   Renderer::g_Allocator = NULL;
VkInstance               Renderer::g_Instance = VK_NULL_HANDLE;
VkPhysicalDevice         Renderer::g_PhysicalDevice = VK_NULL_HANDLE;
VkDevice                 Renderer::g_Device = VK_NULL_HANDLE;
uint32_t                 Renderer::g_QueueFamily = (uint32_t)-1;
VkQueue                  Renderer::g_Queue = VK_NULL_HANDLE;
VkDebugReportCallbackEXT Renderer::g_DebugReport = VK_NULL_HANDLE;
VkPipelineCache          Renderer::g_PipelineCache = VK_NULL_HANDLE;
VkDescriptorPool         Renderer::g_DescriptorPool = VK_NULL_HANDLE;

ImGui_ImplVulkanH_Window Renderer::g_MainWindowData;
uint32_t                 Renderer::g_MinImageCount = 2;
bool                     Renderer::g_SwapChainRebuild = false;
bool                     Renderer::validation = false;

VkSurfaceKHR Renderer::vksurface;
ImGui_ImplVulkanH_Window* Renderer::wd;
// end moved over

bool Renderer::sdlInitialised = false;
SDL_WindowFlags Renderer::sdlWindowFlags;
SDL_Window* Renderer::pSdlWindow;

std::vector<const char*> Renderer::requiredExtensions;
std::vector<const char*> Renderer::optionalExtensions;

#ifdef IMGUI_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    (void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
    fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
    return VK_FALSE;
}
#endif // IMGUI_VULKAN_DEBUG_REPORT

Renderer::Renderer()
{}

Renderer::~Renderer()
{}

EError Renderer::SdlInit()
{
    assert(!sdlInitialised);
    LOGINFO("Renderer::SdlInit()");

    // Create an SDL window that supports Vulkan rendering.
    if (SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        return ErrorLogAndReturn(EError::SDL_UnableToInitialize);
    }

    sdlWindowFlags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    pSdlWindow = SDL_CreateWindow("Prometheus", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, sdlWindowFlags);

    if (pSdlWindow == NULL)
    {
        return ErrorLogAndReturn(EError::SDL_CouldNotCreateWindow);
    }

    // Get WSI extensions from SDL (we can add more if we like - we just can't
    // remove these)
    unsigned extensionCount;
    if (!SDL_Vulkan_GetInstanceExtensions(pSdlWindow, &extensionCount, NULL))
    {
        return ErrorLogAndReturn(EError::SDL_CouldNotGetRequiredVulkanExtensions);
    }

    std::vector<const char*> sdlExtensions;
    sdlExtensions.resize(extensionCount);

    if (!SDL_Vulkan_GetInstanceExtensions(pSdlWindow, &extensionCount, sdlExtensions.data()))
    {
        return ErrorLogAndReturn(EError::SDL_CouldNotGetRequiredVulkanExtensions);
    }

    // copy sdl extensions to required extensions

    for(auto ext : sdlExtensions)
        requiredExtensions.push_back(ext);

    sdlInitialised = true;
	return EError::OK;
}

void Renderer::EnableValidation()
{
	const std::vector<const char*> requiredLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	for(auto requiredLayer : requiredLayers)
	{
		bool supportsLayer = false;
		for(auto layerProperties : availableLayers)
		{
			if(strcmp(layerProperties.layerName, requiredLayer) == 0)
			{
				supportsLayer = true;
			}
		}

		if(supportsLayer)
		{
			enabledLayers.push_back(requiredLayer);
		}
		else
		{
			LOGFATALF("Vulkan::Does not support required %s layer", requiredLayer);
		}
	}
}

void Renderer::GetRequiredAndOptionalExtensions()
{
	// TODO: Move the SDL extension injection to here

	// if validation is enabled we need to add the message callback extension
	if(validation)
	{
		optionalExtensions.push_back("VK_EXT_DEBUG_UTILS_EXTENSION_NAME");
	}
}

bool Renderer::CheckRequiredExtensions()
{
	// go through required extensions and check they exist in the available extensions list
	for(auto requiredExt : requiredExtensions)
	{
		bool thisOneAvailable = false;
		for(auto availableExt : availableExtensions)
		{
			if(strcmp(availableExt.extensionName, requiredExt) == 0)
			{
				thisOneAvailable = true;
			}
		}
		if(!thisOneAvailable)
		{
			// report the error and bail
			LOGFATALF("Vulkan::Required extension is not available: %s", requiredExt);
			return false;
		}
	}

	// now go through optional extensions and try to survive if they are not available
	std::vector<std::string> removeFromOptional;
	for(auto optionalExt : optionalExtensions)
	{
		bool thisOneAvailable = false;
		for(auto availableExt : availableExtensions)
		{
			if(strcmp(availableExt.extensionName, optionalExt) == 0)
			{
				thisOneAvailable = true;
			}
		}
		if(!thisOneAvailable)
		{
			if(strcmp(optionalExt, "VK_EXT_DEBUG_UTILS_EXTENSION_NAME") == 0)
			{
				// turn off validation
				LOGWARNING("Vulkan::Validation extension not available - disabling validation. (VK_EXT_DEBUG_UTILS_EXTENSION_NAME)");
				validation = false;
			}
			else
			{
				LOGWARNINGF("Vulkan::Extension not available %s", optionalExt);
			}
			removeFromOptional.push_back(optionalExt);
		}
	}
	// remove any unavailable optional extensions
	for(std::vector<std::string>::iterator rem = removeFromOptional.begin() ; rem != removeFromOptional.end() ; ++rem)
	{
		for(std::vector<const char*>::iterator iter = optionalExtensions.begin() ; iter != optionalExtensions.end() ; ++iter)
		{
			if(strcmp(*iter, (*rem).c_str()) == 0)
			{
				optionalExtensions.erase(iter);
				break;
			}
		}
	}
	// copy required and remaining optional extensions to the requested extensions list
	for(auto required : requiredExtensions)
	{
		requestedExtensions.push_back(required);
	}
	for(auto optional : optionalExtensions)
	{
		requestedExtensions.push_back(optional);
	}
	return true;
}

void Renderer::Initialise(SDL_Window* window)
{
    Renderer::SetupVulkan(window);

    // Create Window Surface
    if (SDL_Vulkan_CreateSurface(window, Renderer::g_Instance, &Renderer::vksurface) == 0)
    {
        printf("Failed to create Vulkan surface.\n");
        exit(-1);
    }

    // Create Framebuffers
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    Renderer::wd = &Renderer::g_MainWindowData;
    Renderer::SetupVulkanWindow(Renderer::wd, Renderer::vksurface, w, h);

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
    init_info.Instance = Renderer::g_Instance;
    init_info.PhysicalDevice = Renderer::g_PhysicalDevice;
    init_info.Device = Renderer::g_Device;
    init_info.QueueFamily = Renderer::g_QueueFamily;
    init_info.Queue = Renderer::g_Queue;
    init_info.PipelineCache = Renderer::g_PipelineCache;
    init_info.DescriptorPool = Renderer::g_DescriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = Renderer::g_MinImageCount;
    init_info.ImageCount = Renderer::wd->ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = Renderer::g_Allocator;
    init_info.CheckVkResultFn = Renderer::CheckVkResult;
    ImGui_ImplVulkan_Init(&init_info, Renderer::wd->RenderPass);

//    LoadFonts();
    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Upload Fonts
    {
        // Use any command queue
        VkCommandPool command_pool = Renderer::wd->Frames[Renderer::wd->FrameIndex].CommandPool;
        VkCommandBuffer command_buffer = Renderer::wd->Frames[Renderer::wd->FrameIndex].CommandBuffer;

        VkResult err = vkResetCommandPool(Renderer::g_Device, command_pool, 0);
        Renderer::CheckVkResult(err);
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(command_buffer, &begin_info);
        Renderer::CheckVkResult(err);

        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

        VkSubmitInfo end_info = {};
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &command_buffer;
        err = vkEndCommandBuffer(command_buffer);
        Renderer::CheckVkResult(err);
        err = vkQueueSubmit(Renderer::g_Queue, 1, &end_info, VK_NULL_HANDLE);
        Renderer::CheckVkResult(err);

        err = vkDeviceWaitIdle(Renderer::g_Device);
        Renderer::CheckVkResult(err);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
}

void Renderer::Cleanup()
{
    VkResult err = vkDeviceWaitIdle(Renderer::g_Device);
    CheckVkResult(err);
    ImGui_ImplVulkan_Shutdown();
    CleanupVulkanWindow();
    CleanupVulkan();
}

void Renderer::SetupVulkan(SDL_Window* window)
{
	validation = Config::Instance()->GetBool("vulkan.instance.validation");

    VkResult err;

	// get layers

	// Get extensions required by SDL and add them to the required list
    uint32_t extensions_count = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &extensions_count, NULL);
    const char** extensions = new const char*[extensions_count];
    SDL_Vulkan_GetInstanceExtensions(window, &extensions_count, extensions);



    // Create Vulkan Instance
    {
        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.enabledExtensionCount = extensions_count;
        create_info.ppEnabledExtensionNames = extensions;
#ifdef IMGUI_VULKAN_DEBUG_REPORT
        // Enabling validation layers
        const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = layers;

        // Enable debug report extension (we need additional storage, so we duplicate the user array to add our new extension to it)
        const char** extensions_ext = (const char**)malloc(sizeof(const char*) * (extensions_count + 1));
        memcpy(extensions_ext, extensions, extensions_count * sizeof(const char*));
        extensions_ext[extensions_count] = "VK_EXT_debug_report";
        create_info.enabledExtensionCount = extensions_count + 1;
        create_info.ppEnabledExtensionNames = extensions_ext;

        // Create Vulkan Instance
        err = vkCreateInstance(&create_info, Renderer::g_Allocator, &Renderer::g_Instance);
        Renderer::CheckVkResult(err);
        free(extensions_ext);

        // Get the function pointer (required for any extensions)
        auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(Renderer::g_Instance, "vkCreateDebugReportCallbackEXT");
        IM_ASSERT(vkCreateDebugReportCallbackEXT != NULL);

        // Setup the debug report callback
        VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
        debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debug_report_ci.pfnCallback = debug_report;
        debug_report_ci.pUserData = NULL;
        err = vkCreateDebugReportCallbackEXT(Renderer::g_Instance, &debug_report_ci, Renderer::g_Allocator, &Renderer::g_DebugReport);
        Renderer::CheckVkResult(err);
#else
        // Create Vulkan Instance without any debug feature
        err = vkCreateInstance(&create_info, Renderer::g_Allocator, &Renderer::g_Instance);
        Renderer::CheckVkResult(err);
        IM_UNUSED(Renderer::g_DebugReport);
#endif
    }

    delete[] extensions;

    // Select GPU
    {
        uint32_t gpu_count;
        err = vkEnumeratePhysicalDevices(Renderer::g_Instance, &gpu_count, NULL);
        Renderer::CheckVkResult(err);
        IM_ASSERT(gpu_count > 0);

        VkPhysicalDevice* gpus = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpu_count);
        err = vkEnumeratePhysicalDevices(Renderer::g_Instance, &gpu_count, gpus);
        Renderer::CheckVkResult(err);

        // If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
        // most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
        // dedicated GPUs) is out of scope of this sample.
        int use_gpu = 0;
        for (int i = 0; i < (int)gpu_count; i++)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(gpus[i], &properties);
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                use_gpu = i;
                break;
            }
        }

        Renderer::g_PhysicalDevice = gpus[use_gpu];
        free(gpus);
    }

    // Select graphics queue family
    {
        uint32_t count;
        vkGetPhysicalDeviceQueueFamilyProperties(Renderer::g_PhysicalDevice, &count, NULL);
        VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * count);
        vkGetPhysicalDeviceQueueFamilyProperties(Renderer::g_PhysicalDevice, &count, queues);
        for (uint32_t i = 0; i < count; i++)
            if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                Renderer::g_QueueFamily = i;
                break;
            }
        free(queues);
        IM_ASSERT(Renderer::g_QueueFamily != (uint32_t)-1);
    }

    // Create Logical Device (with 1 queue)
    {
        int device_extension_count = 1;
        const char* device_extensions[] = { "VK_KHR_swapchain" };
        const float queue_priority[] = { 1.0f };
        VkDeviceQueueCreateInfo queue_info[1] = {};
        queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[0].queueFamilyIndex = Renderer::g_QueueFamily;
        queue_info[0].queueCount = 1;
        queue_info[0].pQueuePriorities = queue_priority;
        VkDeviceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
        create_info.pQueueCreateInfos = queue_info;
        create_info.enabledExtensionCount = device_extension_count;
        create_info.ppEnabledExtensionNames = device_extensions;
        err = vkCreateDevice(Renderer::g_PhysicalDevice, &create_info, Renderer::g_Allocator, &Renderer::g_Device);
        Renderer::CheckVkResult(err);
        vkGetDeviceQueue(Renderer::g_Device, Renderer::g_QueueFamily, 0, &Renderer::g_Queue);
    }

    // Create Descriptor Pool
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
        err = vkCreateDescriptorPool(Renderer::g_Device, &pool_info, Renderer::g_Allocator, &Renderer::g_DescriptorPool);
        Renderer::CheckVkResult(err);
    }
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
void Renderer::SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height)
{
    wd->Surface = surface;

    // Check for WSI support
    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(Renderer::g_PhysicalDevice, Renderer::g_QueueFamily, wd->Surface, &res);
    if (res != VK_TRUE)
    {
        fprintf(stderr, "Error no WSI support on physical device 0\n");
        exit(-1);
    }

    // Select Surface Format
    const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(Renderer::g_PhysicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

    // Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
    VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
    VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(Renderer::g_PhysicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
    //printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    IM_ASSERT(Renderer::g_MinImageCount >= 2);
    ImGui_ImplVulkanH_CreateOrResizeWindow(Renderer::g_Instance, Renderer::g_PhysicalDevice, Renderer::g_Device, wd, Renderer::g_QueueFamily, Renderer::g_Allocator, width, height, Renderer::g_MinImageCount);
}

void Renderer::CleanupVulkan()
{
    vkDestroyDescriptorPool(Renderer::g_Device, Renderer::g_DescriptorPool, Renderer::g_Allocator);

#ifdef IMGUI_VULKAN_DEBUG_REPORT
    // Remove the debug report callback
    auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(Renderer::g_Instance, "vkDestroyDebugReportCallbackEXT");
    vkDestroyDebugReportCallbackEXT(Renderer::g_Instance, Renderer::g_DebugReport, Renderer::g_Allocator);
#endif // IMGUI_VULKAN_DEBUG_REPORT

    vkDestroyDevice(Renderer::g_Device, Renderer::g_Allocator);
    vkDestroyInstance(Renderer::g_Instance, Renderer::g_Allocator);
}

void Renderer::CleanupVulkanWindow()
{
    ImGui_ImplVulkanH_DestroyWindow(Renderer::g_Instance, Renderer::g_Device, &Renderer::g_MainWindowData, Renderer::g_Allocator);
}

void Renderer::BeginFrame(SDL_Window* window)
{
	// Resize swap chain?
	if (Renderer::g_SwapChainRebuild)
	{
		int width, height;
		SDL_GetWindowSize(window, &width, &height);
		if (width > 0 && height > 0)
		{
			ImGui_ImplVulkan_SetMinImageCount(Renderer::g_MinImageCount);
			ImGui_ImplVulkanH_CreateOrResizeWindow(Renderer::g_Instance, Renderer::g_PhysicalDevice, Renderer::g_Device, &Renderer::g_MainWindowData, Renderer::g_QueueFamily, Renderer::g_Allocator, width, height, Renderer::g_MinImageCount);
			Renderer::g_MainWindowData.FrameIndex = 0;
			Renderer::g_SwapChainRebuild = false;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame();
}


void Renderer::FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data)
{
    VkResult err;

    VkSemaphore image_acquired_semaphore  = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    err = vkAcquireNextImageKHR(Renderer::g_Device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        Renderer::g_SwapChainRebuild = true;
        return;
    }
    Renderer::CheckVkResult(err);

    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    {
        err = vkWaitForFences(Renderer::g_Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
        Renderer::CheckVkResult(err);

        err = vkResetFences(Renderer::g_Device, 1, &fd->Fence);
        Renderer::CheckVkResult(err);
    }
    {
        err = vkResetCommandPool(Renderer::g_Device, fd->CommandPool, 0);
        Renderer::CheckVkResult(err);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        Renderer::CheckVkResult(err);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
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
        Renderer::CheckVkResult(err);
        err = vkQueueSubmit(Renderer::g_Queue, 1, &info, fd->Fence);
        Renderer::CheckVkResult(err);
    }
}

void Renderer::FramePresent(ImGui_ImplVulkanH_Window* wd)
{
    if (Renderer::g_SwapChainRebuild)
        return;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    VkResult err = vkQueuePresentKHR(Renderer::g_Queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        Renderer::g_SwapChainRebuild = true;
        return;
    }
    Renderer::CheckVkResult(err);
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount; // Now we can use the next set of semaphores
}

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

EError Renderer::Shutdown()
{
    vkDestroySurfaceKHR(instance, surface, NULL);
    SDL_DestroyWindow(pSdlWindow);
    SDL_Quit();
    vkDestroyInstance(instance, NULL);

    return EError::OK;
}

void Renderer::LogInstanceProperties()
{
    LOGINFO("=== Vulkan instance info ===");
    LOGINFO("");
    LOGINFO("--- Application info ---");
    LOGINFO("");
    LOGINFOF("Application name:%s", applicationInfo.pApplicationName);
    LOGINFOF("Application version %d", applicationInfo.applicationVersion);
    LOGINFOF("Engine name:%s", applicationInfo.pEngineName);
    LOGINFOF("Engine version:%d", applicationInfo.engineVersion);
    LOGINFOF("API version:%s", StringAPIVersion(applicationInfo.apiVersion).c_str());
    LOGINFOF("Num physical devices:%d", physicalDevices.size());

    if(Config::Instance()->GetBool("vulkan.instance.loginfo.extensions", true))
    {
        LOGINFO("");
        LOGINFO("--- Supported extensions ---");
        LOGINFO("");
        for(int iExtension=0 ; iExtension<availableExtensions.size() ; iExtension++)
        {
            LOGINFOF("Extension: %s spec: %d", availableExtensions[iExtension].extensionName, availableExtensions[iExtension].specVersion);
        }
    }

    if(Config::Instance()->GetBool("vulkan.instance.loginfo.extensions", true))
    {
        LOGINFO("");
        LOGINFO("--- Supported layers ---");
        LOGINFO("");
        for(int iLayer=0 ; iLayer<availableLayers.size() ; iLayer++)
        {
            LOGINFOF("Layer: %s spec: %d", availableLayers[iLayer].layerName, availableLayers[iLayer].specVersion);
        }
    }
}

void Renderer::CheckVkResult(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}
