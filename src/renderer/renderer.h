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
#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "system/errors.h"

class SDL_Window;
class VulkanInstance;
class VulkanLogicalDevice;
class VulkanPhysicalDevice;

class Renderer
{
public:
	Renderer();
	virtual ~Renderer();

	// TODO: Setters, getters
    VkClearValue        	clearValue;

	void Initialise(SDL_Window* window);
	void BeginFrame();
    void FramePresent();
	void Cleanup();

	// Debug
	void DrawVulkanDebugWindow();

	// IMGUI
    void ImGuiRender(ImDrawData* draw_data);

private:
	struct FrameData
	{
		VkCommandPool       CommandPool;
		VkCommandBuffer     CommandBuffer;
		VkFence             Fence;
		VkImage             Backbuffer;
		VkImageView         BackbufferView;
		VkFramebuffer       Framebuffer;
	};

	struct FrameSemaphores
	{
		VkSemaphore         ImageAcquiredSemaphore;
		VkSemaphore         RenderCompleteSemaphore;
	};

    VkAllocationCallbacks*   vkAllocatorCallbacks;
	VulkanInstance*			instance;

	// TODO: change to smart pointer ?
	VulkanPhysicalDevice*		physicalDevice;
	VulkanLogicalDevice* 		device;

    uint32_t                 vkQueueGraphicsFamily;
    VkQueue                  vkGraphicsQueue;
    VkDebugReportCallbackEXT vkDebugReport;
    VkPipelineCache          vkPipelineCache;
    VkDescriptorPool         vkDescriptorPool;
    VkSwapchainKHR      	swapchain;
    VkSurfaceFormatKHR  	windowSurfaceFormat;
    VkPresentModeKHR    	windowPresentMode;
    bool                	windowClearEnable;
    uint32_t            	windowFrameIndex;             // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
    VkRenderPass        	renderPass;
    VkPipeline          	pipeline;               // The window pipeline may uses a different VkRenderPass than the one passed in ImGui_ImplVulkan_InitInfo
    uint32_t            	semaphoreIndex;         // Current set of swapchain wait semaphores we're using (needs to be distinct from per frame data)
    FrameData*				frames;
    FrameSemaphores*  		frameSemaphores;
    uint32_t				minImageCount;
	uint32_t				windowImageCount;
    bool					swapChainRebuild;
	int						windowWidth;
	int						windowHeight;
    VkSurfaceKHR			windowSurface;

	SDL_Window* sdlWindow;
	std::vector<VulkanPhysicalDevice*> physicalDevices;

    std::vector<const char*> requiredDeviceExtensions;

	bool validation;

/**
 * @brief Main setup method
 * 
 */
    void Setup();

/**
 * @brief Plap
 * 
 * @param width 
 * @param height 
 */
    void SetupVulkanWindow(int width, int height);
    void CleanupVulkan();
    void CleanupVulkanWindow();
	void CreateOrResizeWindow(uint32_t width, uint32_t height);
	void SetupDebugReportCallback();
	void SetupPhysicalDevice();
	void SetupQueueFamilies();
	void SetupLogicalDevice();
	void SetupDescriptorPool();

	// IMGUI
	void ImGuiSetup();
};
