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

// OLD
#if 0
    EError Init();

    EError Shutdown();
	bool Validation(){ return validation; }
#endif
// ENDOLD

    // moved over from imgui sample...

    static void CheckVkResult(VkResult err);

    VkAllocationCallbacks*   vkAllocatorCallbacks;
    VkInstance               vkInstance;
    VkPhysicalDevice         vkPhysicalDevice;
    VkDevice                 vkDevice;
    uint32_t                 vkQueueFamily;
    VkQueue                  vkQueue;
    VkDebugReportCallbackEXT vkDebugReport;
    VkPipelineCache          vkPipelineCache;
    VkDescriptorPool         vkDescriptorPool;

    ImGui_ImplVulkanH_Window mainWindowData;
    uint32_t                 minImageCount;
    bool                     swapChainRebuild;

    VkSurfaceKHR vkSurface;
    ImGui_ImplVulkanH_Window* wd;

	void Initialise(SDL_Window* window);
	void Cleanup();

	void BeginFrame(SDL_Window* window);

    void SetupVulkan(SDL_Window* window);
    void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height);
    void CleanupVulkan();
    void CleanupVulkanWindow();
    void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data);
    void FramePresent(ImGui_ImplVulkanH_Window* wd);

//private:
	bool validation;

    // SDL

    static bool sdlInitialised;
    static SDL_WindowFlags sdlWindowFlags;
    static SDL_Window* pSdlWindow;

    // Main

	std::vector<VkLayerProperties> availableInstanceLayers;
    std::vector<const char*> requiredInstanceLayers;

    std::vector<const char*> requiredExtensions;
    std::vector<const char*> optionalExtensions;

// OLD
#if 0
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


    std::vector<const char*> requestedExtensions;
    std::vector<VkExtensionProperties> availableExtensions;

    std::vector<RendererPhysicalDevice> physicalDevices;
	int	chosenPhysicalDevice;
	std::shared_ptr<RendererLogicalDevice> pLogicalDevice;
#endif
// ENDOLD
};
