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

    EError Init();
    EError Shutdown();
	bool Validation(){ return validation; }

private:
	bool validation;

    EError InitSDL();

    void LogInstanceProperties();
	void EnableValidation();
	void GetRequiredAndOptionalExtensions();
	bool CheckRequiredExtensions();

    SDL_Window *window;

    VkInstance instance;
    VkSurfaceKHR surface;

    VkApplicationInfo applicationInfo;
    VkInstanceCreateInfo instanceInfo;

    std::vector<const char *> enabledLayers;
    std::vector<VkLayerProperties> availableLayers;

    std::vector<const char*> requiredExtensions;
    std::vector<const char*> optionalExtensions;

    std::vector<const char*> requestedExtensions;
    std::vector<VkExtensionProperties> availableExtensions;

    std::vector<RendererPhysicalDevice> physicalDevices;
	int	chosenPhysicalDevice;
	std::shared_ptr<RendererLogicalDevice> pLogicalDevice;
};
