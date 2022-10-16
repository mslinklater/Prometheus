#include <stdio.h>
#include <string.h>

#include "rendererlogicaldevice.h"
#include "system/log.h"

RendererLogicalDevice::RendererLogicalDevice(VkPhysicalDevice physicalDeviceIn)
{
	physicalDevice = physicalDeviceIn;

	// select which queues we are wanting to use

	VkDeviceQueueCreateInfo queueCreateInfo[1];
	queueCreateInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo[0].pNext = nullptr;
//	queueCreateInfo[0].

	// TODO: Setup the createInfo and allocationCallbacks

	memset(&createInfo, 1, sizeof(VkDeviceCreateInfo));
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	//createInfo.queueCreateInfoCount;
	//createInfo.pQueueCreateInfos = 
	//createInfo.enableLayerCount;
	//createInfo.ppEnabledLayerNames;
	//createInfo.enabledExtensionCount;
	//createInfo.ppEnabledExtensionNames;
	//createInfo.pEnabledFeatures;

	// TODO - setup the rest of 

	if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
	{
		LOGFATAL("Unable to create logical device...");
	}
}
