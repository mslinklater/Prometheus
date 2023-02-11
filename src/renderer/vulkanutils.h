#include <vector>
#include <string>
#include <vulkan/vulkan.h>

void CheckVkResult(VkResult err);

namespace VulkanUtils
{
	VkSurfaceFormatKHR FindBestSurfaceFormat(	VkPhysicalDevice device, 
												std::vector<VkFormat>& requestedFormats,
												VkColorSpaceKHR requestedColorSpace,
												VkSurfaceKHR surface);

	VkPresentModeKHR FindBestPresentMode(	VkPhysicalDevice device, 
											VkSurfaceKHR surface, 
											std::vector<VkPresentModeKHR>& requestedModes);

	std::string VendorIDToString(uint32_t vendorID);
}
