#pragma once

#include <map>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace ShaderModuleManager
{
	VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code, const std::string& name);
	void DestroyShaderModule(VkDevice device, VkShaderModule module);
	std::string GetShaderModuleName(VkShaderModule module);

}
