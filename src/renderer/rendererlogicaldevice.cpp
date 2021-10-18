#include <stdio.h>
#include <string.h>

#include "rendererlogicaldevice.h"

RendererLogicalDevice::RendererLogicalDevice(VkPhysicalDevice physicalDeviceIn)
{
	physicalDevice = physicalDeviceIn;

	// TODO: Setup the createInfo and allocationCallbacks

	memset(&createInfo, 1, sizeof(VkDeviceCreateInfo));
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = nullptr;


	vkCreateDevice(physicalDevice, &createInfo, &allocationCallbacks, &device);
}
