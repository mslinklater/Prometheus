#include <stdio.h>
#include <string.h>

#include "rendererlogicaldevice.h"
#include "system/log.h"

RendererLogicalDevice::RendererLogicalDevice(VkDevice logicalDeviceIn)
{
	device = logicalDeviceIn;
}
