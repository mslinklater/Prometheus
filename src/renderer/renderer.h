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

#include <vulkan/vulkan.h>

#include "system/errors.h"

#include "rendererphysicaldevice.h"

class SDL_Window;

class Renderer
{
  public:
    Renderer() {}
    virtual ~Renderer() {}

    EError Init();
    EError Shutdown();

  private:
    void LogPhysicalDeviceProperties(VkPhysicalDeviceProperties *pDevice);

    SDL_Window *window;

    VkInstance instance;
    VkSurfaceKHR surface;

    VkApplicationInfo appInfo;
    VkInstanceCreateInfo instInfo;

    std::vector<const char *> layers;
    std::vector<const char *> extensions;

    std::vector<RendererPhysicalDevice> physicalDevices;
};
