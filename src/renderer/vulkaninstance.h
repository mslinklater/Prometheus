#pragma once
#include <vector>
#include <vulkan/vulkan.h>

class SDL_Window;

class VulkanInstance
{
public:
	VulkanInstance(VkAllocationCallbacks* callbacks, bool validation, SDL_Window* sdlWindow);
	virtual ~VulkanInstance(){}

	VkInstance GetVkInstance(){return instance;}

	void DrawDebug();

private:
	void SetupInstanceLayers(bool validation);
	void SetupInstanceExtensions(bool validation, SDL_Window* sdlWindow);

	static bool initialised;
	VkInstance instance;

	uint32_t instanceVersion;
	uint32_t instanceVersionParts[4];

	std::vector<VkLayerProperties> availableInstanceLayers;
    std::vector<const char*> requiredInstanceLayers;
	
	std::vector<VkExtensionProperties> instanceExtensionProperties;
    std::vector<const char*> requiredInstanceExtensions;
};
