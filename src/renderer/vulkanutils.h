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

	std::string ValidationObjectTypeToString(uint32_t objectType);

	enum class ValidationReportType
	{
		Unknown,
		Warning,
		Error
	};
	enum class ValidationWarningType
	{
		Unknown,
		Performance
	};

	VKAPI_ATTR VkBool32 VKAPI_CALL ValidationReport(	VkDebugReportFlagsEXT flags, 
													VkDebugReportObjectTypeEXT objectType, 
													uint64_t object, 
													size_t location, 
													int32_t messageCode, 
													const char* pLayerPrefix, 
													const char* pMessage, 
													void* pUserData);

}
