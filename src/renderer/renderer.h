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
//#include "backends/imgui_impl_vulkan.h"
#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "system/errors.h"

class SDL_Window;
class RendererLogicalDevice;
class RendererPhysicalDevice;

#if 1

struct ImGui_Frame
{
    VkCommandPool       CommandPool;
    VkCommandBuffer     CommandBuffer;
    VkFence             Fence;
    VkImage             Backbuffer;
    VkImageView         BackbufferView;
    VkFramebuffer       Framebuffer;
};

struct ImGui_FrameSemaphores
{
    VkSemaphore         ImageAcquiredSemaphore;
    VkSemaphore         RenderCompleteSemaphore;
};

struct ImGui_Window
{
    int                 Width;
    int                 Height;
    VkSwapchainKHR      Swapchain;
    VkSurfaceKHR        Surface;
    VkSurfaceFormatKHR  SurfaceFormat;
    VkPresentModeKHR    PresentMode;
    VkRenderPass        RenderPass;
    VkPipeline          Pipeline;               // The window pipeline may uses a different VkRenderPass than the one passed in ImGui_ImplVulkan_InitInfo
    bool                ClearEnable;
    VkClearValue        ClearValue;
    uint32_t            FrameIndex;             // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
    uint32_t            ImageCount;             // Number of simultaneous in-flight frames (returned by vkGetSwapchainImagesKHR, usually derived from min_image_count)
    uint32_t            SemaphoreIndex;         // Current set of swapchain wait semaphores we're using (needs to be distinct from per frame data)
    ImGui_Frame*            Frames;
    ImGui_FrameSemaphores*  FrameSemaphores;

    ImGui_Window()
    {
        memset((void*)this, 0, sizeof(*this));
        PresentMode = (VkPresentModeKHR)~0;     // Ensure we get an error if user doesn't set this.
        ClearEnable = true;
    }
};
#endif

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

    ImGui_Window imguiVulkanWindowData;	// TODO: Remove

    uint32_t                 minImageCount;
    bool                     swapChainRebuild;

    VkSurfaceKHR vkSurface;
    ImGui_Window* imguiWindow;	// TODO: Remove

	void Initialise(SDL_Window* window);
	void Cleanup();

	void BeginFrame();

    void Setup();
	// TODO: refactor out
    void SetupVulkanWindow(ImGui_Window* imguiWindow, VkSurfaceKHR surface, int width, int height);
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
