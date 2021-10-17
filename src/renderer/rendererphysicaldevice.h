#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class RendererPhysicalDevice
{
public:
	RendererPhysicalDevice(){}
	virtual ~RendererPhysicalDevice(){}

	void SetPhysicalDevice(VkPhysicalDevice deviceIn);


private:
	VkPhysicalDevice device;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
};
