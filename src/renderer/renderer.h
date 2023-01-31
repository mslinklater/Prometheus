// [+Header]
// [-Header]

#pragma once

// Enable the WSI extensions
#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <memory>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_vulkan.h"
#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "system/errors.h"

#include "rendererphysicaldevice.h"

class SDL_Window;
class RendererLogicalDevice;

class Renderer
{
public:
	Renderer();
	virtual ~Renderer();

    // SDL API

    static EError SdlInit();
    static SDL_WindowFlags SdlGetWindowFlags(){ return sdlWindowFlags; }
    static SDL_Window* SdlGetWindowPtr(){ return pSdlWindow; }

    // Main API

    static const char**    GetRequiredExtensions(){ return requiredExtensions.data(); }
    static uint32_t        GetRequiredExtensionsCount(){ return requiredExtensions.size(); }

    // old
    EError Init();

    EError Shutdown();
	bool Validation(){ return validation; }

    static void CheckVkResult(VkResult err);

    // moved over from imgui sample...

    static VkAllocationCallbacks*   g_Allocator;
    static VkInstance               g_Instance;
    static VkPhysicalDevice         g_PhysicalDevice;
    static VkDevice                 g_Device;
    static uint32_t                 g_QueueFamily;
    static VkQueue                  g_Queue;
    static VkDebugReportCallbackEXT g_DebugReport;
    static VkPipelineCache          g_PipelineCache;
    static VkDescriptorPool         g_DescriptorPool;

    static ImGui_ImplVulkanH_Window g_MainWindowData;
    static uint32_t                 g_MinImageCount;
    static bool                     g_SwapChainRebuild;

private:
	bool validation;

    // SDL

    static bool sdlInitialised;
    static SDL_WindowFlags sdlWindowFlags;
    static SDL_Window* pSdlWindow;

    // Main

    void LogInstanceProperties();
	void EnableValidation();
	void GetRequiredAndOptionalExtensions();
	bool CheckRequiredExtensions();

    VkInstance instance;
    VkSurfaceKHR surface;

    VkApplicationInfo applicationInfo;
    VkInstanceCreateInfo instanceInfo;

    std::vector<const char *> enabledLayers;
    std::vector<VkLayerProperties> availableLayers;

    static std::vector<const char*> requiredExtensions;
    static std::vector<const char*> optionalExtensions;

    std::vector<const char*> requestedExtensions;
    std::vector<VkExtensionProperties> availableExtensions;

    std::vector<RendererPhysicalDevice> physicalDevices;
	int	chosenPhysicalDevice;
	std::shared_ptr<RendererLogicalDevice> pLogicalDevice;
};
