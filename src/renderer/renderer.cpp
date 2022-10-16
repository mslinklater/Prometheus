#include <iostream>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#include "vulkan/vk_layer_utils.h"

#include "rendererlogicaldevice.h"
#include "renderer.h"
#include "system/config.h"
#include "system/log.h"

EError Renderer::InitSDL()
{
    // Create an SDL window that supports Vulkan rendering.
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        return ErrorLogAndReturn(EError::SDL_UnableToInitialize);
    }

    window = SDL_CreateWindow("Prometheus", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN);

    if (window == NULL)
    {
        return ErrorLogAndReturn(EError::SDL_CouldNotCreateWindow);
    }

    // Get WSI extensions from SDL (we can add more if we like - we just can't
    // remove these)
    unsigned extension_count;
    if (!SDL_Vulkan_GetInstanceExtensions(window, &extension_count, NULL))
    {
        return ErrorLogAndReturn(EError::SDL_CouldNotGetRequiredVulkanExtensions);
    }

    enabledExtensions.resize(extension_count);

    if (!SDL_Vulkan_GetInstanceExtensions(window, &extension_count, enabledExtensions.data()))
    {
        return ErrorLogAndReturn(EError::SDL_CouldNotGetRequiredVulkanExtensions);
    }
	
	return EError::OK;
}

EError Renderer::Init()
{
    LOGVERBOSE("Renderer:Init()");

	InitSDL();

    // Get available instance layers

    uint32_t numAvailableLayers;
    vkEnumerateInstanceLayerProperties(&numAvailableLayers, nullptr);
    availableLayers.resize(numAvailableLayers);
    vkEnumerateInstanceLayerProperties(&numAvailableLayers, &availableLayers[0]);

    // Get available instance extensions

    uint32_t numAvailableExtensions;
    vkEnumerateInstanceExtensionProperties(nullptr, &numAvailableExtensions, nullptr);
    availableExtensions.resize(numAvailableExtensions);
    vkEnumerateInstanceExtensionProperties(nullptr, &numAvailableExtensions, &availableExtensions[0]);

    // Create instance

#if defined(DEBUG)
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    // VkApplicationInfo allows the programmer to specifiy some basic
    // information about the program, which can be useful for layers and tools
    // to provide more debug information.
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = NULL;
    applicationInfo.pApplicationName = "Prometheus";
    applicationInfo.applicationVersion = 1;
    applicationInfo.pEngineName = "Prometheus";
    applicationInfo.engineVersion = 1;
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    // VkInstanceCreateInfo is where the programmer specifies the layers and/or
    // extensions that are needed.
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = NULL;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &applicationInfo;
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    instanceInfo.ppEnabledExtensionNames = enabledExtensions.data();
    instanceInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
    instanceInfo.ppEnabledLayerNames = enabledLayers.data();

    // Create the Vulkan instance.
    VkResult result = vkCreateInstance(&instanceInfo, NULL, &instance);
    if (result == VK_ERROR_INCOMPATIBLE_DRIVER)
    {
        return ErrorLogAndReturn(EError::Vulkan_UnableToFindDriver);
    }
    else if (result)
    {
        return ErrorLogAndReturn(EError::Vulkan_CouldNotCreateInstance);
    }
	LOGINFO("Vulkan::Instance created");
    
    // Enumerate physical devices

    uint32_t physicalDeviceCount;
    result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    if (result)
    {
        return ErrorLogAndReturn(EError::Vulkan_CouldNotEnumeratePhysicalDevices);
    }

    physicalDevices.resize(physicalDeviceCount);

    std::vector<VkPhysicalDevice> vulkanPhysicalDevices(physicalDeviceCount);

    result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, &vulkanPhysicalDevices[0]);
    if (result)
    {
        return ErrorLogAndReturn(EError::Vulkan_CouldNotEnumeratePhysicalDevices);
    }

    for (uint32_t iDevice = 0; iDevice < physicalDeviceCount; iDevice++)
    {
        physicalDevices[iDevice].SetPhysicalDevice(vulkanPhysicalDevices[iDevice]);
		physicalDeviceAcceptable.push_back(true);
    }

    if(Config::Instance()->GetBool("vulkan.instance.loginfo"))
    {
        LogInstanceProperties();
    }
    if(Config::Instance()->GetBool("vulkan.devices.loginfo"))
    {
        for (int iDevice = 0; iDevice < physicalDevices.size(); iDevice++)
        {
            physicalDevices[iDevice].LogDeviceInfo();
        }
    }

    // TODO: choose which physical device to use and create the logical device

	// go through acceptablePhysicalDevices vector and test against desired properties, removing ones which don't satisfy requirements
    for (uint32_t iDevice = 0; iDevice < physicalDeviceCount; iDevice++)
	{
		if(physicalDevices[iDevice].GraphicsQueueIndex() == -1)
        {
            physicalDeviceAcceptable[iDevice] = false;
        }
    }

    // check for a preferred vulkan device
    if (Config::Instance()->StringExists("vulkan.device.preferred"))
    {
        // find the preferred device
        std::string preferred = Config::Instance()->GetString("vulkan.device.preferred");
    }
    else
    {
        // find the best device with required caps
    }

    // Create a Vulkan surface for rendering
    if (!SDL_Vulkan_CreateSurface(window, instance, &surface))
    {
        return ErrorLogAndReturn(EError::SDL_CouldNotCreateVulkanSurface);
    }

	// Create logical device

	pLogicalDevice = std::make_shared<RendererLogicalDevice>(physicalDevices[0].GetPhysicalDevice());
//	pLogicalDevice->

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

void Renderer::LogInstanceProperties()
{
    LOGINFO("=== Vulkan instance info ===");
    LOGINFO("");
    LOGINFO("--- Application info ---");
    LOGINFO("");
    LOGINFOF("Application name:%s", applicationInfo.pApplicationName);
    LOGINFOF("Application version %d", applicationInfo.applicationVersion);
    LOGINFOF("Engine name:%s", applicationInfo.pEngineName);
    LOGINFOF("Engine version:%d", applicationInfo.engineVersion);
    LOGINFOF("API version:%s", StringAPIVersion(applicationInfo.apiVersion).c_str());
    LOGINFOF("Num physical devices:%d", physicalDevices.size());

    if(Config::Instance()->GetBool("vulkan.instance.loginfo.extensions", true))
    {
        LOGINFO("");
        LOGINFO("--- Supported extensions ---");
        LOGINFO("");
        for(int iExtension=0 ; iExtension<availableExtensions.size() ; iExtension++)
        {
            LOGINFOF("Extension: %s spec: %d", availableExtensions[iExtension].extensionName, availableExtensions[iExtension].specVersion);
        }
    }

    if(Config::Instance()->GetBool("vulkan.instance.loginfo.extensions", true))
    {
        LOGINFO("");
        LOGINFO("--- Supported layers ---");
        LOGINFO("");
        for(int iLayer=0 ; iLayer<availableLayers.size() ; iLayer++)
        {
            LOGINFOF("Layer: %s spec: %d", availableLayers[iLayer].layerName, availableLayers[iLayer].specVersion);
        }
    }
}

