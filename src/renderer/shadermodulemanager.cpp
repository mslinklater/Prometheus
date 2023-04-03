#include "shadermodulemanager.h"

#include "system/log.h"

static std::map<VkShaderModule, std::string> moduleToNameMap;

VkShaderModule ShaderModuleManager::CreateShaderModule(VkDevice device, const std::vector<char>& code, const std::string& name)
{
	VkShaderModuleCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		LOGERROR("vkCreateShaderModule failed");
	}

	moduleToNameMap[shaderModule] = name;

	return shaderModule;
}

void ShaderModuleManager::DestroyShaderModule(VkDevice device, VkShaderModule module)
{
	vkDestroyShaderModule(device, module, nullptr);
	moduleToNameMap.erase(module);
}

std::string ShaderModuleManager::GetShaderModuleName(VkShaderModule module)
{
	// TODO: Needs error handling
	return moduleToNameMap[module];
}

