#include <iostream>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#include "renderer.h"
#include "log.h"

EError Renderer::Init()
{
	LOGINFO("Renderer:Init()");

    // Create an SDL window that supports Vulkan rendering.
    if(SDL_Init(SDL_INIT_VIDEO) != 0) 
    {
		return ErrorLogAndReturn(EError::SDL_UnableToInitialize);
    }

    window = SDL_CreateWindow("FaffAboutEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN);

    if(window == NULL) 
    {
		return ErrorLogAndReturn(EError::SDL_CouldNotCreateWindow);
    }

    // Get WSI extensions from SDL (we can add more if we like - we just can't remove these)
    unsigned extension_count;
    if(!SDL_Vulkan_GetInstanceExtensions(window, &extension_count, NULL)) 
    {
		return ErrorLogAndReturn(EError::SDL_CouldNotGetRequiredVulkanExtensions);
    }

	extensions.resize(extension_count);

    if(!SDL_Vulkan_GetInstanceExtensions(window, &extension_count, extensions.data())) 
    {
		return ErrorLogAndReturn(EError::SDL_CouldNotGetRequiredVulkanExtensions);
    }

	// Create instance

#if defined(DEBUG)
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    // VkApplicationInfo allows the programmer to specifiy some basic information about the
    // program, which can be useful for layers and tools to provide more debug information.
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = NULL;
    appInfo.pApplicationName = "FaffAboutEngine";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "FaffAbout";
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // VkInstanceCreateInfo is where the programmer specifies the layers and/or extensions that
    // are needed.
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pNext = NULL;
    instInfo.flags = 0;
    instInfo.pApplicationInfo = &appInfo;
    instInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instInfo.ppEnabledExtensionNames = extensions.data();
    instInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    instInfo.ppEnabledLayerNames = layers.data();

    // Create the Vulkan instance.
    VkResult result = vkCreateInstance(&instInfo, NULL, &instance);
    if(result == VK_ERROR_INCOMPATIBLE_DRIVER) 
	{
		return ErrorLogAndReturn(EError::Vulkan_UnableToFindDriver);
    } 
	else if(result) 
	{
		return ErrorLogAndReturn(EError::Vulkan_CouldNotCreateInstance);
    }

	// Enumerate physical devices

	uint32_t physicalDeviceCount;
	result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
	if(result)
	{
		return ErrorLogAndReturn(EError::Vulkan_CouldNotEnumeratePhysicalDevices);
	}

	physicalDevices.resize(physicalDeviceCount);

	std::vector<VkPhysicalDevice> vulkanPhysicalDevices(physicalDeviceCount);

	result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, &vulkanPhysicalDevices[0]);
	if(result)
	{
		return ErrorLogAndReturn(EError::Vulkan_CouldNotEnumeratePhysicalDevices);
	}

	for(uint32_t iDevice=0 ; iDevice < physicalDeviceCount ; iDevice++)
	{
		physicalDevices[iDevice].SetPhysicalDevice(vulkanPhysicalDevices[iDevice]);
		physicalDevices[iDevice].LogDeviceInfo();
	}

	// TODO: choose which physical device to use and create the logical device
	

    // Create a Vulkan surface for rendering
    if(!SDL_Vulkan_CreateSurface(window, instance, &surface)) 
	{
		return ErrorLogAndReturn(EError::SDL_CouldNotCreateVulkanSurface);
    }

	return EError::OK;
}

EError Renderer::Shutdown()
{
    vkDestroySurfaceKHR(instance, surface, NULL);
    SDL_DestroyWindow(window);
    SDL_Quit();
    vkDestroyInstance(instance, NULL);

	return EError::OK;
}

void Renderer::LogPhysicalDeviceProperties(VkPhysicalDeviceProperties* pProperties)
{

}
