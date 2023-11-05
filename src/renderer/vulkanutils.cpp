#include <iostream>
#include <sstream>

#include "vulkanutils.h"
#include "system/log.h"

#include "shadermodulemanager.h"

void CheckVkResult(VkResult err)
{
    if (err == 0)
        return;
	switch(err)
	{
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			std::cerr << "[vulkan] Error: VkResult = VK_ERROR_INCOMPATIBLE_DRIVER";
			break;
		default:
		    std::cerr << "[vulkan] Error: VkResult = " << err;
	}
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

std::string ValidationObjectTypeToString(uint32_t objectType)
{
	switch(objectType)
	{
        case VK_OBJECT_TYPE_QUERY_POOL:
            return "VK_OBJECT_TYPE_QUERY_POOL";
        case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION:
            return "VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION";
        case VK_OBJECT_TYPE_SEMAPHORE:
            return "VK_OBJECT_TYPE_SEMAPHORE";
        case VK_OBJECT_TYPE_SHADER_MODULE:
            return "VK_OBJECT_TYPE_SHADER_MODULE";
        case VK_OBJECT_TYPE_SWAPCHAIN_KHR:
            return "VK_OBJECT_TYPE_SWAPCHAIN_KHR";
        case VK_OBJECT_TYPE_SAMPLER:
            return "VK_OBJECT_TYPE_SAMPLER";
        case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV:
            return "VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV";
        case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:
            return "VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT";
        case VK_OBJECT_TYPE_IMAGE:
            return "VK_OBJECT_TYPE_IMAGE";
        case VK_OBJECT_TYPE_UNKNOWN:
            return "VK_OBJECT_TYPE_UNKNOWN";
        case VK_OBJECT_TYPE_DESCRIPTOR_POOL:
            return "VK_OBJECT_TYPE_DESCRIPTOR_POOL";
        case VK_OBJECT_TYPE_COMMAND_BUFFER:
            return "VK_OBJECT_TYPE_COMMAND_BUFFER";
        case VK_OBJECT_TYPE_BUFFER:
            return "VK_OBJECT_TYPE_BUFFER";
        case VK_OBJECT_TYPE_SURFACE_KHR:
            return "VK_OBJECT_TYPE_SURFACE_KHR";
        case VK_OBJECT_TYPE_INSTANCE:
            return "VK_OBJECT_TYPE_INSTANCE";
        case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT:
            return "VK_OBJECT_TYPE_VALIDATION_CACHE_EXT";
        case VK_OBJECT_TYPE_IMAGE_VIEW:
            return "VK_OBJECT_TYPE_IMAGE_VIEW";
        case VK_OBJECT_TYPE_DESCRIPTOR_SET:
            return "VK_OBJECT_TYPE_DESCRIPTOR_SET";
        case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:
            return "VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT";
        case VK_OBJECT_TYPE_COMMAND_POOL:
            return "VK_OBJECT_TYPE_COMMAND_POOL";
        case VK_OBJECT_TYPE_PHYSICAL_DEVICE:
            return "VK_OBJECT_TYPE_PHYSICAL_DEVICE";
        case VK_OBJECT_TYPE_DISPLAY_KHR:
            return "VK_OBJECT_TYPE_DISPLAY_KHR";
        case VK_OBJECT_TYPE_BUFFER_VIEW:
            return "VK_OBJECT_TYPE_BUFFER_VIEW";
        case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:
            return "VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT";
        case VK_OBJECT_TYPE_FRAMEBUFFER:
            return "VK_OBJECT_TYPE_FRAMEBUFFER";
        case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE:
            return "VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE";
        case VK_OBJECT_TYPE_PIPELINE_CACHE:
            return "VK_OBJECT_TYPE_PIPELINE_CACHE";
        case VK_OBJECT_TYPE_PIPELINE_LAYOUT:
            return "VK_OBJECT_TYPE_PIPELINE_LAYOUT";
        case VK_OBJECT_TYPE_DEVICE_MEMORY:
            return "VK_OBJECT_TYPE_DEVICE_MEMORY";
        case VK_OBJECT_TYPE_FENCE:
            return "VK_OBJECT_TYPE_FENCE";
        case VK_OBJECT_TYPE_QUEUE:
            return "VK_OBJECT_TYPE_QUEUE";
        case VK_OBJECT_TYPE_DEVICE:
            return "VK_OBJECT_TYPE_DEVICE";
        case VK_OBJECT_TYPE_RENDER_PASS:
            return "VK_OBJECT_TYPE_RENDER_PASS";
        case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:
            return "VK_OBJECT_TYPE_DISPLAY_MODE_KHR";
        case VK_OBJECT_TYPE_EVENT:
            return "VK_OBJECT_TYPE_EVENT";
        case VK_OBJECT_TYPE_PIPELINE:
            return "VK_OBJECT_TYPE_PIPELINE";
        default:
            return "Unhandled VkObjectType";	
	}
}

std::string ValidationReportExtractReasonString(const std::string& report)
{
	std::string::size_type start = report.find("[");
	std::string::size_type end = report.find("]");

	return report.substr(start + 2, end - start - 2);
}

std::string ValidationReportExtractDetailString(const std::string& report)
{
	std::string::size_type end = report.rfind("|");

	return report.substr(end + 2);
}

std::string ValidationReportExtractObjectNameString(const std::string& report, uint32_t objectType)
{
	std::string::size_type start = report.find("handle = ");
	std::string handleString = report.substr(start + 11, 16);

	uint64_t x;   
	std::stringstream ss;
	ss << std::hex << handleString;
	ss >> x;

	std::string ret = "unknown";

	switch(objectType)
	{
		case VK_OBJECT_TYPE_SHADER_MODULE:
			ret = ShaderModuleManager::GetShaderModuleName(*((VkShaderModule*)&x));		// filthy hack - do this better
			break;
		default:
			break;
	}

	return ret;
}

VKAPI_ATTR VkBool32 VKAPI_CALL ValidationReport(	VkDebugReportFlagsEXT flags, 
													VkDebugReportObjectTypeEXT objectType, 
													uint64_t object, 
													size_t location, 
													int32_t messageCode, 
													const char* pLayerPrefix, 
													const char* pMessage, 
													void* pUserData)
{
    (void)flags; 
	(void)object; 
	(void)location; 
	(void)messageCode; 
	(void)pUserData; 
	(void)pLayerPrefix; // Unused arguments

	std::string strMessage = pMessage;
	ValidationReportType reportType = ValidationReportType::Unknown;
	ValidationWarningType warningType = ValidationWarningType::Unknown;
	if(strMessage.find("Warning:") != std::string::npos)
	{
		reportType = ValidationReportType::Warning;
		if(strMessage.find("Performance Warning:") != std::string::npos)
		{
			warningType = ValidationWarningType::Performance;
		}
	}
	if(strMessage.find("Error:") != std::string::npos)
	{
		reportType = ValidationReportType::Error;
	}

	std::string objectTypeStr = VulkanUtils::ValidationObjectTypeToString(objectType);

	switch(reportType)
	{
		case ValidationReportType::Warning:
			{
				std::string reason = ValidationReportExtractReasonString(pMessage);
				std::string detail = ValidationReportExtractDetailString(pMessage);
				std::string objectName = ValidationReportExtractObjectNameString(pMessage, objectType);

				switch(warningType)
				{
					case ValidationWarningType::Performance:
						LOGWARNING("[vulkan] --- PERFORMANCE WARNING ---");
						LOGWARNINGF("[vulkan] from: %s", objectTypeStr.c_str());
						LOGWARNINGF("[vulkan] name: %s", objectName.c_str());
						LOGWARNINGF("[vulkan] reason: %s", reason.c_str());
						LOGWARNINGF("[vulkan] detail: %s", detail.c_str());
						LOGINFO("---");
						LOGINFOF("[vulkan] message: %s", pMessage);
						LOGINFO("---");
						break;
					default:
						LOGWARNINGF("[vulkan] Validation WARNING from %s Message: %s", objectTypeStr.c_str(), pMessage);
						break;
				}
			}
			break;
		case ValidationReportType::Error:
			{
				switch(objectType)
				{
					case VK_OBJECT_TYPE_RENDER_PASS:
						{
							std::string reason = ValidationReportExtractReasonString(pMessage);
							std::string detail = ValidationReportExtractDetailString(pMessage);
							std::string objectName = ValidationReportExtractObjectNameString(pMessage, objectType);

						}
						break;
					default:
						LOGERRORF("[vulkan] Validation ERROR from %s Message: %s", objectTypeStr.c_str(), pMessage);
						break;
				}


			}
			break;
		default:
			LOGERRORF("[vulkan] UNKNOWN validation report from %s Message: %s", objectTypeStr.c_str(), pMessage);
			break;
	}

    return VK_FALSE;
}

}	// namespace

