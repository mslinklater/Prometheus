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
// OLD
	Renderer();
	virtual ~Renderer();
// ENDOLD

    // SDL API

    static EError SdlInit();
    static SDL_WindowFlags SdlGetWindowFlags(){ return sdlWindowFlags; }
    static SDL_Window* SdlGetWindowPtr(){ return pSdlWindow; }

    // Main API

//    static const char**    GetRequiredExtensions(){ return requiredExtensions.data(); }
//    static uint32_t        GetRequiredExtensionsCount(){ return requiredExtensions.size(); }

// OLD
#if 0
    EError Init();

    EError Shutdown();
	bool Validation(){ return validation; }
#endif
// ENDOLD

    // moved over from imgui sample...

    static void CheckVkResult(VkResult err);

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

    static VkSurfaceKHR vksurface;
    static ImGui_ImplVulkanH_Window* wd;

	static void Initialise(SDL_Window* window);
	static void Cleanup();

	static void BeginFrame(SDL_Window* window);

    static void SetupVulkan(SDL_Window* window);
    static void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height);
    static void CleanupVulkan();
    static void CleanupVulkanWindow();
    static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data);
    static void FramePresent(ImGui_ImplVulkanH_Window* wd);

private:
	static bool validation;

    // SDL

    static bool sdlInitialised;
    static SDL_WindowFlags sdlWindowFlags;
    static SDL_Window* pSdlWindow;

    // Main
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

    static std::vector<const char*> requiredExtensions;
    static std::vector<const char*> optionalExtensions;

    std::vector<const char*> requestedExtensions;
    std::vector<VkExtensionProperties> availableExtensions;

    std::vector<RendererPhysicalDevice> physicalDevices;
	int	chosenPhysicalDevice;
	std::shared_ptr<RendererLogicalDevice> pLogicalDevice;
#endif
// ENDOLD
};
