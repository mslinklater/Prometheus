#include <assert.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#include "vulkaninstance.h"
#include "rendererutils.h"

#include "system/log.h"
#include "system/config.h"

#include "imgui.h"

bool VulkanInstance::initialised = false;

VulkanInstance::VulkanInstance(VkAllocationCallbacks* vkAllocatorCallbacks, bool validation, SDL_Window* sdlWindow)
{
	assert(!initialised);

    VkResult err;
	// Log Vulkan Instance version
	err = vkEnumerateInstanceVersion(&instanceVersion);
	instanceVersionParts[0] = VK_API_VERSION_VARIANT(instanceVersion);
	instanceVersionParts[1] = VK_API_VERSION_MAJOR(instanceVersion);
	instanceVersionParts[2] = VK_API_VERSION_MINOR(instanceVersion);
	instanceVersionParts[3] = VK_API_VERSION_PATCH(instanceVersion);
	CheckVkResult(err);

	SetupInstanceLayers(validation);
	SetupInstanceExtensions(validation, sdlWindow);

    // Create Vulkan Instance
	VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = NULL;
    applicationInfo.pApplicationName = Config::GetString("application.name").c_str();
    applicationInfo.applicationVersion = 1;
    applicationInfo.pEngineName = "Prometheus";
    applicationInfo.engineVersion = 1;
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledExtensionCount = requiredInstanceExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredInstanceExtensions.data();
    createInfo.enabledLayerCount = requiredInstanceLayers.size();
    createInfo.ppEnabledLayerNames = requiredInstanceLayers.data();

	// Issue#17 - output which extensions & layers are actually requested

	// Create Vulkan Instance
    err = vkCreateInstance(&createInfo, vkAllocatorCallbacks, &instance);
    CheckVkResult(err);

	initialised = true;
}

void VulkanInstance::DrawDebug()
{
		ImGui::Text("Vulkan driver version %d.%d.%d.%d", instanceVersionParts[0], instanceVersionParts[1], instanceVersionParts[2], instanceVersionParts[3]);
        if (ImGui::TreeNode("Layers"))
        {
			if (ImGui::TreeNode("Enabled"))
			{
				for(auto i : requiredInstanceLayers)
				{
					ImGui::Text("%s",i);
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Available"))
			{
				for(auto i : availableInstanceLayers)
				{
					ImGui::Text("%s", &i.layerName[0]);
				}
				ImGui::TreePop();
			}
            ImGui::TreePop();
		}
        if (ImGui::TreeNode("Extensions"))
        {
			if (ImGui::TreeNode("Enabled"))
			{
				for(auto i : requiredInstanceExtensions)
				{
					ImGui::Text("%s",i);
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Available"))
			{
				for(auto i : instanceExtensionProperties)
				{
					ImGui::Text("%s %d", &i.extensionName[0], i.specVersion);
				}
				ImGui::TreePop();
			}
            ImGui::TreePop();
		}

}

void VulkanInstance::SetupInstanceLayers(bool validation)
{
	// Get available instance layers layers
    uint32_t numAvailableInstanceLayers;

    vkEnumerateInstanceLayerProperties(&numAvailableInstanceLayers, nullptr);
    availableInstanceLayers.resize(numAvailableInstanceLayers);
    vkEnumerateInstanceLayerProperties(&numAvailableInstanceLayers, &availableInstanceLayers[0]);

	if(Config::GetBool("vulkan.instance.loginfo.layers"))
	{
		LOGINFO("Vulkan::Available instance layers...");
		for(auto layerinfo : availableInstanceLayers)
		{
			LOGINFOF("   %s", layerinfo.layerName);
		}
	}

	if(validation)
	{
		requiredInstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
	}

	// Check that all required layers are available
	for(auto requiredLayer : requiredInstanceLayers)
	{
		bool found = false;
		for(auto availableLayer : availableInstanceLayers)
		{
			if(strcmp(requiredLayer, availableLayer.layerName) == 0)
			{
				found = true;
			}
		}
		if(!found)
		{
			LOGERRORF("Vulkan::Required instance layer not available: %s", requiredLayer);
		}
	}
}

void VulkanInstance::SetupInstanceExtensions(bool validation, SDL_Window* sdlWindow)
{
	if(Config::GetBool("vulkan.instance.loginfo.extensions"))
	{
		uint32_t numInstanceExtensions;
		vkEnumerateInstanceExtensionProperties(nullptr, &numInstanceExtensions, nullptr);
		instanceExtensionProperties.resize(numInstanceExtensions);
		vkEnumerateInstanceExtensionProperties(nullptr, &numInstanceExtensions, instanceExtensionProperties.data());
		LOGINFO("Vulkan::Available instance extensions...");
		for(auto extension : instanceExtensionProperties)
		{
			LOGINFOF("   %s(%d)", extension.extensionName, extension.specVersion);
		}
	}

	// Get extensions required by SDL and add them to the required list
    uint32_t sdlExtensionsCount = 0;
    SDL_Vulkan_GetInstanceExtensions(sdlWindow, &sdlExtensionsCount, NULL);
	std::vector<const char*> sdlExtensions(sdlExtensionsCount);
    SDL_Vulkan_GetInstanceExtensions(sdlWindow, &sdlExtensionsCount, sdlExtensions.data());
	for(int iExtension = 0 ; iExtension < sdlExtensionsCount ; ++iExtension)
	{
		requiredInstanceExtensions.push_back(sdlExtensions[iExtension]);
	}

	if(validation)
	{
		requiredInstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	//TODO: Check all required extensions are in fact available
}
