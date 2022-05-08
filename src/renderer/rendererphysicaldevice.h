#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class RendererPhysicalDevice
{
  public:
    RendererPhysicalDevice() {}
    virtual ~RendererPhysicalDevice() {}

    void SetPhysicalDevice(VkPhysicalDevice deviceIn);
	VkPhysicalDevice GetPhysicalDevice(){ return device; }

    void LogDeviceName();
    void LogDeviceInfo();

  private:
    VkPhysicalDevice device;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    std::vector<VkLayerProperties> supportedLayers;
	std::vector<VkExtensionProperties> supportedExtensions;
};
