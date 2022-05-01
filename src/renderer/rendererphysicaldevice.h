#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class RendererPhysicalDevice
{
  public:
    RendererPhysicalDevice() {}
    virtual ~RendererPhysicalDevice() {}

    void SetPhysicalDevice(VkPhysicalDevice deviceIn);

    void LogDeviceName();
    void LogDeviceInfo();

  private:
    VkPhysicalDevice device;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    std::vector<VkLayerProperties> layers;
};
