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

    EError Init();
    EError Shutdown();
	bool Validation(){ return validation; }

    static void CheckVkResult(VkResult err);

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
