#pragma once

#include <vulkan/vulkan.h>

class RendererLogicalDevice
{
public:
	RendererLogicalDevice(VkDevice device);
	virtual ~RendererLogicalDevice(){}

	VkDevice GetVkDevice(){return device;}
private:
	VkDevice device;
};
