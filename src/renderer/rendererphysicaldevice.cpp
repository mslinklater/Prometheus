#include "rendererphysicaldevice.h"
#include "system/log.h"

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

void RendererPhysicalDevice::LogDeviceName()
{
	LOGINFOF("Vulkan physical device: %s", properties.deviceName);
}

void RendererPhysicalDevice::LogDeviceInfo()
{
	LOGINFO("--- Vulkan Physical Device Info ---");
	LOGINFOF("Name: %s", properties.deviceName);

	for(int iQueue=0 ; iQueue<queueFamilyProperties.size() ; iQueue++)
	{
		LOGINFO("   --- Queue family ---");
		LOGINFOF("Count: %d", queueFamilyProperties[iQueue].queueCount);
		if(queueFamilyProperties[iQueue].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			LOGINFO("   Graphics");
		}
		if(queueFamilyProperties[iQueue].queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			LOGINFO("   Compute");
		}
		if(queueFamilyProperties[iQueue].queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			LOGINFO("   Transfer");
		}
		if(queueFamilyProperties[iQueue].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
		{
			LOGINFO("   Sparse Binding");
		}
		if(queueFamilyProperties[iQueue].queueFlags & VK_QUEUE_PROTECTED_BIT)
		{
			LOGINFO("   Protected");
		}
	}
	LOGINFO(" ");
}
