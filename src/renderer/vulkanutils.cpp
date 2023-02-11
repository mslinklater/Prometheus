#include <iostream>

#include "vulkanutils.h"

void CheckVkResult(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

namespace VulkanUtils
{

VkSurfaceFormatKHR FindBestSurfaceFormat(VkPhysicalDevice device, std::vector<VkFormat>& requestedFormats, VkColorSpaceKHR requestedColorSpace,VkSurfaceKHR surface)
{
	uint32_t avail_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &avail_count, nullptr);
	std::vector<VkSurfaceFormatKHR> avail_format(avail_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &avail_count, avail_format.data());

	// First check if only one format, VK_FORMAT_UNDEFINED, is available, which would imply that any format is available
	if (avail_count == 1)
	{
		if (avail_format[0].format == VK_FORMAT_UNDEFINED)
		{
			VkSurfaceFormatKHR ret;
			ret.format = requestedFormats[0];
			ret.colorSpace = requestedColorSpace;
			return ret;
		}
		else
		{
			// No point in searching another format
			return avail_format[0];
		}
	}
	else
	{
		// Request several formats, the first found will be used
		// just setting the first one in-case none off the list are found

		for (int request_i = 0; request_i < requestedFormats.size() ; request_i++)
		{
			for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
			{
				if (avail_format[avail_i].format == requestedFormats[request_i] && avail_format[avail_i].colorSpace == requestedColorSpace)
				{
					return avail_format[avail_i];
				}
			}
		}

		return avail_format[0];
	}
}

VkPresentModeKHR FindBestPresentMode(	VkPhysicalDevice device, 
										VkSurfaceKHR surface, 
										std::vector<VkPresentModeKHR>& requestedModes)
{
    uint32_t availCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &availCount, nullptr);
	std::vector<VkPresentModeKHR> availModes(availCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &availCount, availModes.data());

    for (int request_i = 0; request_i < requestedModes.size(); request_i++)
	{
        for (uint32_t avail_i = 0; avail_i < availCount; avail_i++)
		{
            if (requestedModes[request_i] == availModes[avail_i])
			{
                return requestedModes[request_i];
			}
		}
	}

    return VK_PRESENT_MODE_FIFO_KHR; // Always available
}

std::string VendorIDToString(uint32_t vendorID)
{
	switch(vendorID)
	{
		case VK_VENDOR_ID_VIV:	return "viv"; break;
		case VK_VENDOR_ID_VSI:	return "vsi"; break;
		case VK_VENDOR_ID_KAZAN:	return "kazan"; break;
		case VK_VENDOR_ID_CODEPLAY:	return "codeplay"; break;
		case VK_VENDOR_ID_MESA:	return "mesa"; break;
		case VK_VENDOR_ID_POCL:	return "pocl"; break;
		case 0x8086: return "Intel"; break;
	}
	return "unknown";
}

}	// namespace

