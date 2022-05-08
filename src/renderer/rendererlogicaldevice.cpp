#include <stdio.h>
#include <string.h>

#include "rendererlogicaldevice.h"
#include "system/log.h"

RendererLogicalDevice::RendererLogicalDevice(VkPhysicalDevice physicalDeviceIn)
{
	physicalDevice = physicalDeviceIn;

	// TODO: Setup the createInfo and allocationCallbacks

	memset(&createInfo, 1, sizeof(VkDeviceCreateInfo));
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = nullptr;

	// TODO - setup the rest of createInfo

	if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
	{
		LOGFATAL("Unable to create logical device...");
	}
}
