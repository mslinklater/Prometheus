#pragma once

#include <vulkan/vulkan.h>

class RendererLogicalDevice
{
public:
	RendererLogicalDevice(VkPhysicalDevice device);
	virtual ~RendererLogicalDevice(){}

private:
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkDeviceCreateInfo createInfo;
//	VkAllocationCallbacks allocationCallbacks;
};
