#pragma once

#include <vector>
#include <string>
#include <optional>
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

	const std::string& GetName(){ return name; }

	bool HasGraphicsQueue(){ return graphicsQueueIndex.has_value(); }
    uint32_t GraphicsQueueIndex(){ return graphicsQueueIndex.value(); }
	bool HasComputeQueue(){ return computeQueueIndex.has_value(); }
    uint32_t ComputeQueueIndex(){ return computeQueueIndex.value(); }
	bool HasTransferQueue(){ return transferQueueIndex.has_value(); }
    uint32_t TransferQueueIndex(){ return transferQueueIndex.value(); }

	bool acceptable;

  private:
	
    VkPhysicalDevice device;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;	
    VkPhysicalDeviceMemoryProperties memoryProperties;

    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	std::optional<uint32_t> graphicsQueueIndex;
	std::optional<uint32_t> computeQueueIndex;
	std::optional<uint32_t> transferQueueIndex;

    std::vector<VkLayerProperties> supportedLayers;
	std::vector<VkExtensionProperties> supportedExtensions;

	// storage

	std::string name;
};
