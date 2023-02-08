#include <vector>
#include <vulkan/vulkan.h>

namespace RendererUtils
{
	VkSurfaceFormatKHR FindBestSurfaceFormat(	VkPhysicalDevice device, 
												std::vector<VkFormat>& requestedFormats,
												VkColorSpaceKHR requestedColorSpace,
												VkSurfaceKHR surface);

	VkPresentModeKHR FindBestPresentMode(	VkPhysicalDevice device, 
											VkSurfaceKHR surface, 
											std::vector<VkPresentModeKHR>& requestedModes);

}
