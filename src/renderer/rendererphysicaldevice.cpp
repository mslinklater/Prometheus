#include "rendererphysicaldevice.h"

void RendererPhysicalDevice::SetPhysicalDevice(VkPhysicalDevice deviceIn)
{
	device = deviceIn;

	// get properties
	vkGetPhysicalDeviceProperties(device, &properties);

	// get features
	vkGetPhysicalDeviceFeatures(device, &features);

	// get memory properties
	vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

	// get queue family properties
	uint32_t numQueueFamilies;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, nullptr);
	queueFamilyProperties.resize(numQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, &queueFamilyProperties[0]);
	
}
