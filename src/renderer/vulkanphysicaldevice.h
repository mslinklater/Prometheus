#pragma once

#include <vector>
#include <string>
#include <optional>
#include <vulkan/vulkan.h>

class VulkanPhysicalDevice
{
  public:
    VulkanPhysicalDevice() {}
    virtual ~VulkanPhysicalDevice() {}

    void SetVkPhysicalDevice(VkPhysicalDevice deviceIn);
	VkPhysicalDevice GetVkPhysicalDevice(){ return device; }

    void LogDeviceName();
    void LogDeviceInfo();

	void DrawDebug();

	const std::string& GetName(){ return name; }

	std::optional<uint32_t> const& GetGraphicsQueueIndex() const { return graphicsQueueIndex; }
	std::optional<uint32_t> const& GetComputeQueueIndex() const { return graphicsQueueIndex; }
	std::optional<uint32_t> const& GetTransferQueueIndex() const { return graphicsQueueIndex; }

	bool IsDiscreetGPU();

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
