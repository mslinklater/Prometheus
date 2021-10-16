#include <iostream>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#include "renderer.h"

EError Renderer::Init()
{
    // Create an SDL window that supports Vulkan rendering.
    if(SDL_Init(SDL_INIT_VIDEO) != 0) 
    {
        std::cout << "Could not initialize SDL." << std::endl;
        return EError::SDL_UnableToInitialize;
    }

    window = SDL_CreateWindow("FaffAboutEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN);

    if(window == NULL) 
    {
        std::cout << "Could not create SDL window." << std::endl;
        return EError::SDL_CouldNotCreateWindow;
    }

    // Get WSI extensions from SDL (we can add more if we like - we just can't remove these)
    unsigned extension_count;
    if(!SDL_Vulkan_GetInstanceExtensions(window, &extension_count, NULL)) 
    {
        std::cout << "Could not get the number of required instance extensions from SDL." << std::endl;
        return EError::SDL_CouldNotGetRequiredVulkanExtensions;
    }

	extensions.resize(extension_count);

    if(!SDL_Vulkan_GetInstanceExtensions(window, &extension_count, extensions.data())) 
    {
        std::cout << "Could not get the names of required instance extensions from SDL." << std::endl;
        return EError::SDL_CouldNotGetRequiredVulkanExtensions;
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
        std::cout << "Unable to find a compatible Vulkan Driver." << std::endl;
        return EError::Vulkan_UnableToFindDriver;
    } 
	else if(result) 
	{
        std::cout << "Could not create a Vulkan instance (for unknown reasons)." << std::endl;
        return EError::Vulkan_CouldNotCreateInstance;
    }

	// Enumerate physical devices

	uint32_t physicalDeviceCount;
	result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
	if(result)
	{
		std::cout << "Could not enumerate Vulkan physical device count" << std::endl;
		return EError::Vulkan_CouldNotEnumeratePhysicalDevices;
	}
	physicalDevices.resize(physicalDeviceCount);
	physicalDeviceProperties.resize(physicalDeviceCount);
	result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, &physicalDevices[0]);
	if(result)
	{
		for(uint32_t iDevice=0 ; iDevice < physicalDeviceCount ; iDevice++)
		{
			
		}
	}

    // Create a Vulkan surface for rendering
    if(!SDL_Vulkan_CreateSurface(window, instance, &surface)) 
	{
        std::cout << "Could not create a Vulkan surface." << std::endl;
        return EError::Vulkan_CouldNotCreateSurface;
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
