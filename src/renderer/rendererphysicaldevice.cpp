#include "rendererphysicaldevice.h"
#include "system/log.h"
#include "system/config.h"
#include "vulkan/vk_layer_utils.h"

void RendererPhysicalDevice::SetPhysicalDevice(VkPhysicalDevice deviceIn)
{
	device = deviceIn;
	acceptable = true;

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
	name = properties.deviceName;

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

	// Find the queue type indices
	for(int iQueue = 0 ; iQueue<queueFamilyProperties.size() ; iQueue++)
	{
		if(queueFamilyProperties[iQueue].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
	        graphicsQueueIndex = iQueue;
        }
		if(queueFamilyProperties[iQueue].queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
	        computeQueueIndex = iQueue;
        }
		if(queueFamilyProperties[iQueue].queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
	        transferQueueIndex = iQueue;
        }
	}
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

	if(Config::GetBool("vulkan.devices.loginfo.layers", true))
	{
		LOGINFO("");
		LOGINFO("--- Layers ---");
		LOGINFO("");
		for(int iLayer=0 ; iLayer<supportedLayers.size() ; iLayer++)
		{
			LOGINFOF("Layer: %s spec: %d", supportedLayers[iLayer].layerName, supportedLayers[iLayer].specVersion);
		}
	}

	if(Config::GetBool("vulkan.devices.loginfo.extensions", true))
	{
		LOGINFO("");
		LOGINFO("--- Extensions ---");
		LOGINFO("");
		for(int iExtension=0 ; iExtension<supportedExtensions.size() ; iExtension++)
		{
			LOGINFOF("Extension: %s spec: %d", supportedExtensions[iExtension].extensionName, supportedExtensions[iExtension].specVersion);
		}
	}

	if(Config::GetBool("vulkan.devices.loginfo.limits", true))
	{
		LOGINFO("");
		LOGINFO("--- Limits ---");
		LOGINFO("");
		LOGINFOF("Limit: maxImageDimension1D:%d", properties.limits.maxImageDimension1D);
		LOGINFOF("Limit: maxImageDimension2D:%d", properties.limits.maxImageDimension2D);
		LOGINFOF("Limit: maxImageDimension3D:%d", properties.limits.maxImageDimension3D);
		LOGINFOF("Limit: maxImageDimensionCube:%d", properties.limits.maxImageDimensionCube);
	    LOGINFOF("Limit: maxImageArrayLayers:%d", properties.limits.maxImageArrayLayers);
	    LOGINFOF("Limit: maxTexelBufferElements:%d", properties.limits.maxTexelBufferElements);
	    LOGINFOF("Limit: maxUniformBufferRange:%d", properties.limits.maxUniformBufferRange);
	    LOGINFOF("Limit: maxStorageBufferRange:%d", properties.limits.maxStorageBufferRange);
	    LOGINFOF("Limit: maxPushConstantsSize:%d", properties.limits.maxPushConstantsSize);
	    LOGINFOF("Limit: maxMemoryAllocationCount:%d", properties.limits.maxMemoryAllocationCount);
	    LOGINFOF("Limit: maxSamplerAllocationCount:%d", properties.limits.maxSamplerAllocationCount);
	//    LOGINFOF("Limit: bufferImageGranularity:%d", properties.limits);
	//    LOGINFOF("Limit: sparseAddressSpaceSize:%d", properties.limits);
	    LOGINFOF("Limit: maxBoundDescriptorSets:%d", properties.limits.maxBoundDescriptorSets);
	    LOGINFOF("Limit: maxPerStageDescriptorSamplers:%d", properties.limits.maxPerStageDescriptorSamplers);
	    LOGINFOF("Limit: maxPerStageDescriptorUniformBuffers:%d", properties.limits.maxPerStageDescriptorUniformBuffers);
	    LOGINFOF("Limit: maxPerStageDescriptorStorageBuffers:%d", properties.limits.maxPerStageDescriptorStorageBuffers);
	    LOGINFOF("Limit: maxPerStageDescriptorSampledImages:%d", properties.limits.maxPerStageDescriptorSampledImages);
	    LOGINFOF("Limit: maxPerStageDescriptorStorageImages:%d", properties.limits.maxPerStageDescriptorStorageImages);
	    LOGINFOF("Limit: maxPerStageDescriptorInputAttachments:%d", properties.limits.maxPerStageDescriptorInputAttachments);
	    LOGINFOF("Limit: maxPerStageResources:%d", properties.limits.maxPerStageResources);
	//    LOGINFOF("Limit: :%d", properties.limits);
	//    LOGINFOF("Limit: :%d", properties.limits);
	//    LOGINFOF("Limit: :%d", properties.limits);
	//    LOGINFOF("Limit: :%d", properties.limits);
	//    LOGINFOF("Limit: :%d", properties.limits);
	//    LOGINFOF("Limit: :%d", properties.limits);
	//    LOGINFOF("Limit: :%d", properties.limits);
	//    LOGINFOF("Limit: :%d", properties.limits);
	//    LOGINFOF("Limit: :%d", properties.limits);
	//    LOGINFOF("Limit: :%d", properties.limits);
	}

    // Features
	if(Config::GetBool("vulkan.devices.loginfo.features", true))
	{
		LOGINFO("");
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
		LOGINFOF("Feature: shaderImageGatherExtended:%d", features.shaderImageGatherExtended);
		LOGINFOF("Feature: shaderStorageImageExtendedFormats:%d", features.shaderStorageImageExtendedFormats);
		LOGINFOF("Feature: shaderStorageImageMultisample:%d", features.shaderStorageImageMultisample);
		LOGINFOF("Feature: shaderStorageImageReadWithoutFormat:%d", features.shaderStorageImageReadWithoutFormat);
		LOGINFOF("Feature: shaderStorageImageWriteWithoutFormat:%d", features.shaderStorageImageWriteWithoutFormat);
		LOGINFOF("Feature: shaderUniformBufferArrayDynamicIndexing:%d", features.shaderUniformBufferArrayDynamicIndexing);
		LOGINFOF("Feature: shaderSampledImageArrayDynamicIndexing:%d", features.shaderSampledImageArrayDynamicIndexing);
		LOGINFOF("Feature: shaderStorageBufferArrayDynamicIndexing:%d", features.shaderStorageBufferArrayDynamicIndexing);
		LOGINFOF("Feature: shaderStorageImageArrayDynamicIndexing:%d", features.shaderStorageImageArrayDynamicIndexing);
		LOGINFOF("Feature: shaderClipDistance:%d", features.shaderClipDistance);
		LOGINFOF("Feature: shaderCullDistance:%d", features.shaderCullDistance);
		LOGINFOF("Feature: shaderFloat64:%d", features.shaderFloat64);
		LOGINFOF("Feature: shaderInt64:%d", features.shaderInt64);
		LOGINFOF("Feature: shaderInt16:%d", features.shaderInt16);
		LOGINFOF("Feature: shaderResourceResidency:%d", features.shaderResourceResidency);
		LOGINFOF("Feature: shaderResourceMinLod:%d", features.shaderResourceMinLod);
		LOGINFOF("Feature: sparseBinding:%d", features.sparseBinding);
		LOGINFOF("Feature: sparseResidencyBuffer:%d", features.sparseResidencyBuffer);
		LOGINFOF("Feature: sparseResidencyImage2D:%d", features.sparseResidencyImage2D);
		LOGINFOF("Feature: sparseResidencyImage3D:%d", features.sparseResidencyImage3D);
		LOGINFOF("Feature: sparseResidency2Samples:%d", features.sparseResidency2Samples);
		LOGINFOF("Feature: sparseResidency4Samples:%d", features.sparseResidency4Samples);
		LOGINFOF("Feature: sparseResidency8Samples:%d", features.sparseResidency8Samples);
		LOGINFOF("Feature: sparseResidency16Samples:%d", features.sparseResidency16Samples);
		LOGINFOF("Feature: sparseResidencyAliased:%d", features.sparseResidencyAliased);
		LOGINFOF("Feature: variableMultisampleRate:%d", features.variableMultisampleRate);
		LOGINFOF("Feature: inheritedQueries:%d", features.inheritedQueries);
	}

	// Memory properties
	if(Config::GetBool("vulkan.devices.loginfo.memory", true))
	{
		LOGINFO("");
		LOGINFO("   --- Memory ---");
		LOGINFO("");
		LOGINFOF("Num memory types:%d", memoryProperties.memoryTypeCount);
		LOGINFO("");

		for(int iMemType=0 ; iMemType < memoryProperties.memoryTypeCount ; iMemType++)
		{
			bool bDeviceLocal = memoryProperties.memoryTypes[iMemType].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			bool bHostVisible = memoryProperties.memoryTypes[iMemType].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			bool bHostCoherent = memoryProperties.memoryTypes[iMemType].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			bool bHostCached = memoryProperties.memoryTypes[iMemType].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
			bool bLazilyAllocated = memoryProperties.memoryTypes[iMemType].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
			bool bProtected = memoryProperties.memoryTypes[iMemType].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT;
			bool bDeviceCoherent = memoryProperties.memoryTypes[iMemType].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD;
			bool bDeviceUncached = memoryProperties.memoryTypes[iMemType].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD;
#if defined(VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV)
			bool bRdmaCapable = memoryProperties.memoryTypes[iMemType].propertyFlags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV;
#else
			bool bRdmaCapable = false;
#endif

			LOGINFOF("Heap %d: %s", memoryProperties.memoryTypes[iMemType].heapIndex,
				bDeviceLocal ? "[DeviceLocal] " : "",
				bHostVisible ? "[HostVisible] " : "",
				bHostCoherent ? "[HostCoherent] " : "",
				bHostCached ? "[HostCached] " : "",
				bLazilyAllocated ? "[LazilyAllocated] " : "",
				bProtected ? "[Protected] " : "",
				bDeviceCoherent ? "[DeviceCoherent] " : "",
				bDeviceUncached ? "[DeviceUncached] " : "",
				bRdmaCapable ? "[RDMACapable] " : ""
			);
		}
		LOGINFO("");
		LOGINFOF("Num memory heaps:%d", memoryProperties.memoryHeapCount);
		LOGINFO("");
		for(int iMemHeap=0 ; iMemHeap < memoryProperties.memoryHeapCount ; iMemHeap++)
		{
			bool bLocal = memoryProperties.memoryHeaps[iMemHeap].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
			bool bMultiInstance = memoryProperties.memoryHeaps[iMemHeap].flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT;
			LOGINFOF("Heap %d size:%luMiB %s %s", iMemHeap, memoryProperties.memoryHeaps[iMemHeap].size / (1024 * 1024),
				bLocal ? "[Local]" : "",
				bMultiInstance ? "[MultiInstance]" : ""
			);
		}
	}

	// Queue family properties
	if(Config::GetBool("vulkan.devices.loginfo.queues", true))
	{
		LOGINFO("");
		LOGINFO("   --- Queues ---");
		LOGINFO("");
		for(int iQueue=0 ; iQueue<queueFamilyProperties.size() ; iQueue++)
		{
			LOGINFOF("Queue family %d", iQueue);
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
			LOGINFO("");
		}
	}
}
