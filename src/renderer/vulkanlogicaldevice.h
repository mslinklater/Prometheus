#pragma once

#include <vulkan/vulkan.h>

class VulkanLogicalDevice
{
public:
	VulkanLogicalDevice(VkDevice device);
	virtual ~VulkanLogicalDevice(){}

	VkDevice GetVkDevice(){return device;}
private:
	VkDevice device;
};
