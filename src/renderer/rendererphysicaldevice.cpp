#include "rendererphysicaldevice.h"
#include "system/log.h"
#include "vulkan/vk_layer_utils.h"

void RendererPhysicalDevice::SetPhysicalDevice(VkPhysicalDevice deviceIn)
{
	device = deviceIn;

	// get layers
    uint32_t numLayers;
    vkEnumerateDeviceLayerProperties(device, &numLayers, nullptr);
    supportedLayers.resize(numLayers);
    vkEnumerateDeviceLayerProperties(device, &numLayers, &supportedLayers[0]);

	// get extensions
    uint32_t numExtensions;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &numExtensions, nullptr);
    supportedExtensions.resize(numExtensions);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &numExtensions, &supportedExtensions[0]);

    // get properties
	vkGetPhysicalDeviceProperties(device, &properties);

	// get features
	vkGetPhysicalDeviceFeatures(device, &features);

	// get memory properties
	vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

	// get physical device format properties

	// get queue family properties
	uint32_t numQueueFamilies;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, nullptr);
	queueFamilyProperties.resize(numQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, &queueFamilyProperties[0]);
}

void RendererPhysicalDevice::LogDeviceName()
{
	LOGINFOF("Vulkan physical device: %s", properties.deviceName);
}

void RendererPhysicalDevice::LogDeviceInfo()
{
    LOGINFO("");
	LOGINFO("=== Vulkan Physical Device Info ===");
	LOGINFO("");
	LOGINFO("--- Properties ---");
	LOGINFO("");
	// Properties
	LOGINFOF("Name: %s", properties.deviceName);
    LOGINFOF("API version: %s", StringAPIVersion(properties.apiVersion).c_str());
    LOGINFOF("Driver version: %s", StringAPIVersion(properties.driverVersion).c_str());
    LOGINFOF("Vendor id: %04x", properties.vendorID);
    LOGINFOF("Device id: %04x", properties.deviceID);
	if(properties.deviceType & VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
	{
	    LOGINFO("Device type: Integrated GPU");
	}
	if(properties.deviceType & VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
	    LOGINFO("Device type: Discrete GPU");
	}
	if(properties.deviceType & VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
	{
	    LOGINFO("Device type: Virtual GPU");
	}
	if(properties.deviceType & VK_PHYSICAL_DEVICE_TYPE_CPU)
	{
	    LOGINFO("Device type: CPU");
	}
    LOGINFOF("Device name: %s", properties.deviceName);

	LOGINFO("");
	LOGINFO("--- Layers ---");
	LOGINFO("");
	for(int iLayer=0 ; iLayer<supportedLayers.size() ; iLayer++)
	{
		LOGINFOF("Layer: %s spec: %d", supportedLayers[iLayer].layerName, supportedLayers[iLayer].specVersion);
	}

	LOGINFO("");
	LOGINFO("--- Extensions ---");
	LOGINFO("");
	for(int iExtension=0 ; iExtension<supportedExtensions.size() ; iExtension++)
	{
		LOGINFOF("Extension: %s spec: %d", supportedExtensions[iExtension].extensionName, supportedExtensions[iExtension].specVersion);
	}

	LOGINFO("");
	LOGINFO("--- Limits ---");
	LOGINFO("");

    // Features
	LOGINFO("--- Features ---");
	LOGINFO("");
    LOGINFOF("Feature: robustBufferAccess:%d", features.robustBufferAccess);
    LOGINFOF("Feature: fullDrawIndexUint32:%d", features.fullDrawIndexUint32);
    LOGINFOF("Feature: imageCubeArray:%d", features.imageCubeArray);
    LOGINFOF("Feature: independentBlend:%d", features.independentBlend);
    LOGINFOF("Feature: geometryShader:%d", features.geometryShader);
    LOGINFOF("Feature: tessellationShader:%d", features.tessellationShader);
    LOGINFOF("Feature: sampleRateShading:%d", features.sampleRateShading);
    LOGINFOF("Feature: dualSrcBlend:%d", features.dualSrcBlend);
    LOGINFOF("Feature: logicOp:%d", features.logicOp);
    LOGINFOF("Feature: multiDrawIndirect:%d", features.multiDrawIndirect);
    LOGINFOF("Feature: drawIndirectFirstInstance:%d", features.drawIndirectFirstInstance);
    LOGINFOF("Feature: depthClamp:%d", features.depthClamp);
    LOGINFOF("Feature: depthBiasClamp:%d", features.depthBiasClamp);
    LOGINFOF("Feature: fillModeNonSolid:%d", features.fillModeNonSolid);
    LOGINFOF("Feature: depthBounds:%d", features.depthBounds);
    LOGINFOF("Feature: wideLines:%d", features.wideLines);
    LOGINFOF("Feature: largePoints:%d", features.largePoints);
    LOGINFOF("Feature: alphaToOne:%d", features.alphaToOne);
    LOGINFOF("Feature: multiViewport:%d", features.multiViewport);
    LOGINFOF("Feature: samplerAnisotropy:%d", features.samplerAnisotropy);
    LOGINFOF("Feature: textureCompressionETC2:%d", features.textureCompressionETC2);
    LOGINFOF("Feature: textureCompressionASTC_LDR:%d", features.textureCompressionASTC_LDR);
    LOGINFOF("Feature: textureCompressionBC:%d", features.textureCompressionBC);
    LOGINFOF("Feature: occlusionQueryPrecise:%d", features.occlusionQueryPrecise);
    LOGINFOF("Feature: pipelineStatisticsQuery:%d", features.pipelineStatisticsQuery);
    LOGINFOF("Feature: vertexPipelineStoresAndAtomics:%d", features.vertexPipelineStoresAndAtomics);
    LOGINFOF("Feature: fragmentStoresAndAtomics:%d", features.fragmentStoresAndAtomics);
    LOGINFOF("Feature: shaderTessellationAndGeometryPointSize:%d", features.shaderTessellationAndGeometryPointSize);
#if 0
    VkBool32    shaderImageGatherExtended;
    VkBool32    shaderStorageImageExtendedFormats;
    VkBool32    shaderStorageImageMultisample;
    VkBool32    shaderStorageImageReadWithoutFormat;
    VkBool32    shaderStorageImageWriteWithoutFormat;
    VkBool32    shaderUniformBufferArrayDynamicIndexing;
    VkBool32    shaderSampledImageArrayDynamicIndexing;
    VkBool32    shaderStorageBufferArrayDynamicIndexing;
    VkBool32    shaderStorageImageArrayDynamicIndexing;
    VkBool32    shaderClipDistance;
    VkBool32    shaderCullDistance;
    VkBool32    shaderFloat64;
    VkBool32    shaderInt64;
    VkBool32    shaderInt16;
    VkBool32    shaderResourceResidency;
    VkBool32    shaderResourceMinLod;
    VkBool32    sparseBinding;
    VkBool32    sparseResidencyBuffer;
    VkBool32    sparseResidencyImage2D;
    VkBool32    sparseResidencyImage3D;
    VkBool32    sparseResidency2Samples;
    VkBool32    sparseResidency4Samples;
    VkBool32    sparseResidency8Samples;
    VkBool32    sparseResidency16Samples;
    VkBool32    sparseResidencyAliased;
    VkBool32    variableMultisampleRate;
    VkBool32    inheritedQueries;
#endif

	// Memory properties
	LOGINFO("");
	LOGINFO("   --- Memory ---");
	LOGINFO("");
	LOGINFOF("Num memory types:%d", memoryProperties.memoryTypeCount);
	for(int iMemType=0 ; iMemType < memoryProperties.memoryTypeCount ; iMemType++)
	{
		LOGINFOF("Heap index:%d", memoryProperties.memoryTypes[iMemType].heapIndex);
		if(memoryProperties.memoryTypes[iMemType].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			LOGINFO("Flag: Device Local");
		}
		if(memoryProperties.memoryTypes[iMemType].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		{
			LOGINFO("Flag: Host Visible");
		}
		if(memoryProperties.memoryTypes[iMemType].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
		{
			LOGINFO("Flag: Host Coherent");
		}
		if(memoryProperties.memoryTypes[iMemType].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
		{
			LOGINFO("Flag: Host Cached");
		}
		if(memoryProperties.memoryTypes[iMemType].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
		{
			LOGINFO("Flag: LazilyAllocated");
		}
		if(memoryProperties.memoryTypes[iMemType].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT)
		{
			LOGINFO("Flag: Protected");
		}
		if(memoryProperties.memoryTypes[iMemType].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD)
		{
			LOGINFO("Flag: Device Coherent (AMD)");
		}
		if(memoryProperties.memoryTypes[iMemType].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD)
		{
			LOGINFO("Flag: Device Uncached (AMD)");
		}
		if(memoryProperties.memoryTypes[iMemType].propertyFlags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV)
		{
			LOGINFO("Flag: Device RDMA Capable (NV)");
		}
	}

	LOGINFOF("Num memory heaps:%d", memoryProperties.memoryHeapCount);
	for(int iMemHeap=0 ; iMemHeap < memoryProperties.memoryHeapCount ; iMemHeap++)
	{
		LOGINFOF("Heap %d size:%luMiB", iMemHeap, memoryProperties.memoryHeaps[iMemHeap].size/(1024*1024));
		if(memoryProperties.memoryHeaps[iMemHeap].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
		{
			LOGINFO("Flag: Device Local");
		}
		if(memoryProperties.memoryHeaps[iMemHeap].flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT)
		{
			LOGINFO("Flag: Multi Instance");
		}
	}

	// Queue family properties
	for(int iQueue=0 ; iQueue<queueFamilyProperties.size() ; iQueue++)
	{
		LOGINFO("   --- Queue family ---");
		LOGINFOF("Count: %d", queueFamilyProperties[iQueue].queueCount);
		if(queueFamilyProperties[iQueue].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			LOGINFO("   Graphics");
		}
		if(queueFamilyProperties[iQueue].queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			LOGINFO("   Compute");
		}
		if(queueFamilyProperties[iQueue].queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			LOGINFO("   Transfer");
		}
		if(queueFamilyProperties[iQueue].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
		{
			LOGINFO("   Sparse Binding");
		}
		if(queueFamilyProperties[iQueue].queueFlags & VK_QUEUE_PROTECTED_BIT)
		{
			LOGINFO("   Protected");
		}
	}
	LOGINFO(" ");
}
