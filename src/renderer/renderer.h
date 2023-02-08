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
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_vulkan.h"
#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "system/errors.h"

//#include "rendererphysicaldevice.h"

class SDL_Window;
class RendererLogicalDevice;
class RendererPhysicalDevice;

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

    static void CheckVkResult(VkResult err);

    VkAllocationCallbacks*   vkAllocatorCallbacks;
    VkInstance               vkInstance;
	RendererPhysicalDevice*		physicalDevice;
	RendererLogicalDevice* 		device;
    VkDevice                 vkDevice;
    uint32_t                 vkQueueGraphicsFamily;
    VkQueue                  vkGraphicsQueue;
    VkDebugReportCallbackEXT vkDebugReport;
    VkPipelineCache          vkPipelineCache;
    VkDescriptorPool         vkDescriptorPool;

    ImGui_ImplVulkanH_Window imguiVulkanWindowData;
	
    uint32_t                 minImageCount;
    bool                     swapChainRebuild;

    VkSurfaceKHR vkSurface;
    ImGui_ImplVulkanH_Window* imguiWindow;

	void Initialise(SDL_Window* window);
	void Cleanup();

	void BeginFrame();

    void Setup();
    void SetupVulkanWindow(ImGui_ImplVulkanH_Window* imguiWindow, VkSurfaceKHR surface, int width, int height);
    void CleanupVulkan();
    void CleanupVulkanWindow();
    void FrameRender(ImDrawData* draw_data);
    void FramePresent();

	bool validation;

    // SDL

    static bool sdlInitialised;
    static SDL_WindowFlags sdlWindowFlags;
    static SDL_Window* pSdlWindow;

    // Main

private:
    std::vector<const char*> requiredInstanceLayers;
    std::vector<const char*> requiredInstanceExtensions;
    std::vector<const char*> requiredDeviceExtensions;

private:
	void SetupInstanceLayers();
	void SetupInstanceExtensions();
	void SetupDebugReportCallback();
	void SetupPhysicalDevice();
	void SetupQueueFamilies();
	void SetupLogicalDevice();
	void SetupDescriptorPool();

	SDL_Window* sdlWindow;
	std::vector<RendererPhysicalDevice*> physicalDevices;
};
