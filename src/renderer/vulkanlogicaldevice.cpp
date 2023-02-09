#include <stdio.h>
#include <string.h>

#include "vulkanlogicaldevice.h"
#include "system/log.h"

VulkanLogicalDevice::VulkanLogicalDevice(VkDevice logicalDeviceIn)
{
	device = logicalDeviceIn;
}
