#include <vector>
#include <vulkan/vulkan.h>

void CheckVkResult(VkResult err);

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
