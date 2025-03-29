#include "vulkanphysicaldevice.h"
#include "vulkanutils.h"
#include "system/log.h"
#include "system/config.h"
//#include "vulkan/vk_layer_utils.h"

#include "imgui.h"

void VulkanPhysicalDevice::SetVkPhysicalDevice(VkPhysicalDevice deviceIn)
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
			if(graphicsQueueIndex.has_value())
			{
				LOGWARNING("Found multiple graphics queues - https://github.com/mslinklater/Prometheus/issues/21");
			}
	        graphicsQueueIndex = iQueue;
        }
		if(queueFamilyProperties[iQueue].queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			if(computeQueueIndex.has_value())
			{
				LOGWARNING("Found multiple compute queues - https://github.com/mslinklater/Prometheus/issues/21");
			}
	        computeQueueIndex = iQueue;
        }
		if(queueFamilyProperties[iQueue].queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			if(transferQueueIndex.has_value())
			{
				LOGWARNING("Found multiple transfer queues - https://github.com/mslinklater/Prometheus/issues/21");
			}
	        transferQueueIndex = iQueue;
        }
	}

	LogDeviceInfo();
}

void VulkanPhysicalDevice::DrawDebug()
{
	std::string deviceName = properties.deviceName;
	if (ImGui::TreeNode(deviceName.c_str()))
	{
		switch(properties.deviceType)
		{
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				ImGui::Text("ITEGRATED GPU");
				break;
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				ImGui::Text("DISCRETE GPU");
				break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				ImGui::Text("VIRTUAL GPU");
				break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				ImGui::Text("CPU");
				break;
			default:
				ImGui::Text("OTHER");
				break;
		}

		ImGui::Text("API Version %u", properties.apiVersion);
		ImGui::Text("Driver Version %u", properties.driverVersion);
		ImGui::Text("VendorID 0x%04x %s", properties.vendorID, VulkanUtils::VendorIDToString(properties.vendorID).c_str());
		ImGui::Text("DeviceID 0x%04x", properties.deviceID);
		if (ImGui::TreeNode("Limits"))
		{
			if (ImGui::TreeNode("Descriptor Sets"))
			{
				ImGui::Text("Max bound descriptor sets %d", properties.limits.maxBoundDescriptorSets);
				ImGui::Text("Max descriptor set input attachments %d", properties.limits.maxDescriptorSetInputAttachments);
				ImGui::Text("Max descriptor set sampled images %d", properties.limits.maxDescriptorSetSampledImages);
				ImGui::Text("Max descriptor set samplers %d", properties.limits.maxDescriptorSetSamplers);
				ImGui::Text("Max descriptor set storage buffers %d", properties.limits.maxDescriptorSetStorageBuffers);
				ImGui::Text("Max descriptor set uniform buffers %d", properties.limits.maxDescriptorSetUniformBuffers);
				ImGui::Text("Max descriptor set uniform buffers dynamic %d", properties.limits.maxDescriptorSetUniformBuffersDynamic);
				ImGui::Text("Max per stage descriptor input attachments %d", properties.limits.maxPerStageDescriptorInputAttachments);
				ImGui::Text("Max per stage descriptor sampled images %d", properties.limits.maxPerStageDescriptorSampledImages);
				ImGui::Text("Max per stage descriptor samplers %d", properties.limits.maxPerStageDescriptorSamplers);
				ImGui::Text("Max per stage descriptor storage buffers %d", properties.limits.maxPerStageDescriptorStorageBuffers);
				ImGui::Text("Max per stage descriptor storage images %d", properties.limits.maxPerStageDescriptorStorageImages);
				ImGui::Text("Max per stage descriptor uniform buffers %d", properties.limits.maxPerStageDescriptorUniformBuffers);
			}
			if (ImGui::TreeNode("Compute"))
			{
				ImGui::Text("Max shared memory size %u", properties.limits.maxComputeSharedMemorySize);
				ImGui::Text("Max workgroup count %u, %u, %u", properties.limits.maxComputeWorkGroupCount[0], properties.limits.maxComputeWorkGroupCount[1], properties.limits.maxComputeWorkGroupCount[2]);
				ImGui::Text("Max workgroup invocations %u", properties.limits.maxComputeWorkGroupInvocations);
				ImGui::Text("Max workgroup size %u, %u, %u", properties.limits.maxComputeWorkGroupSize[0], properties.limits.maxComputeWorkGroupSize[1], properties.limits.maxComputeWorkGroupSize[2]);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Fragment"))
			{
				ImGui::Text("Max fragment combined output resources %d", properties.limits.maxFragmentCombinedOutputResources);
				ImGui::Text("Max fragment dual src attachments %d", properties.limits.maxFragmentDualSrcAttachments);
				ImGui::Text("Max fragment input components %d", properties.limits.maxFragmentInputComponents);
				ImGui::Text("Max fragment output attachments %d", properties.limits.maxFragmentOutputAttachments);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Framebuffer"))
			{
				ImGui::Text("Max framebuffer height %d", properties.limits.maxFramebufferHeight);
				ImGui::Text("Max framebuffer layers %d", properties.limits.maxFramebufferLayers);
				ImGui::Text("Max framebuffer width %d", properties.limits.maxFramebufferWidth);
				ImGui::Text("Color sample counts 0x%04x", properties.limits.framebufferColorSampleCounts);
				ImGui::Text("Depth sample counts 0x%04x", properties.limits.framebufferDepthSampleCounts);
				ImGui::Text("No attachments sample counts 0x%04x", properties.limits.framebufferNoAttachmentsSampleCounts);
				ImGui::Text("Stencil sample counts 0x%04x", properties.limits.framebufferStencilSampleCounts);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Geometry"))
			{
				ImGui::Text("Max geometry input components %d", properties.limits.maxGeometryInputComponents);
				ImGui::Text("Max geometry output components %d", properties.limits.maxGeometryOutputComponents);
				ImGui::Text("Max geometry output vertices %d", properties.limits.maxGeometryOutputVertices);
				ImGui::Text("Max geometry shader invocations %d", properties.limits.maxGeometryShaderInvocations);
				ImGui::Text("Max geometry total output components %d", properties.limits.maxGeometryTotalOutputComponents);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Image"))
			{
				ImGui::Text("Max image array layers %d", properties.limits.maxImageArrayLayers);
				ImGui::Text("Max image dimension 1D %d", properties.limits.maxImageDimension1D);
				ImGui::Text("Max image dimension 2D %d", properties.limits.maxImageDimension2D);
				ImGui::Text("Max image dimension 3D %d", properties.limits.maxImageDimension3D);
				ImGui::Text("Max image dimension cube %d", properties.limits.maxImageDimensionCube);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Tesselation"))
			{
				ImGui::Text("Max tesselation control per patch output components %d", properties.limits.maxTessellationControlPerPatchOutputComponents);
				ImGui::Text("Max tesselation control per vertex input components %d", properties.limits.maxTessellationControlPerVertexInputComponents);
				ImGui::Text("Max tesselation control per vertex output components %d", properties.limits.maxTessellationControlPerVertexOutputComponents);
				ImGui::Text("Max tesselation control total output components %d", properties.limits.maxTessellationControlTotalOutputComponents);
				ImGui::Text("Max tesselation evaluation input components 2D %d", properties.limits.maxTessellationEvaluationInputComponents);
				ImGui::Text("Max tesselation evaluation output components 2D %d", properties.limits.maxTessellationEvaluationOutputComponents);
				ImGui::Text("Max tesselation generation level %d", properties.limits.maxTessellationGenerationLevel);
				ImGui::Text("Max tesselation patch size %d", properties.limits.maxTessellationPatchSize);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Vertex"))
			{
				ImGui::Text("Max vertex input attribute offset %d", properties.limits.maxVertexInputAttributeOffset);
				ImGui::Text("Max vertex input attributes %d", properties.limits.maxVertexInputAttributes);
				ImGui::Text("Max vertex input bindings %d", properties.limits.maxVertexInputBindings);
				ImGui::Text("Max vertex input binding stride %d", properties.limits.maxVertexInputBindingStride);
				ImGui::Text("Max vertex output components %d", properties.limits.maxVertexOutputComponents);
				ImGui::TreePop();
			}
			ImGui::Text("Buffer image granularity %llu", properties.limits.bufferImageGranularity);
			ImGui::Text("Discrete queue priorities %u", properties.limits.discreteQueuePriorities);
			ImGui::Text("Line width granularity %f", properties.limits.lineWidthGranularity);
			ImGui::Text("Line width range %f-%f", properties.limits.lineWidthRange[0], properties.limits.lineWidthRange[1]);
			ImGui::Text("Max clip distances %d", properties.limits.maxClipDistances);
			ImGui::Text("Max color attachments %d", properties.limits.maxColorAttachments);
			ImGui::Text("Max combined clip and cull distances %d", properties.limits.maxCombinedClipAndCullDistances);
			ImGui::Text("Max cull distances %d", properties.limits.maxCullDistances);
			ImGui::Text("Max draw indexed index value %d", properties.limits.maxDrawIndexedIndexValue);
			ImGui::Text("Max interpolation offset %f", properties.limits.maxInterpolationOffset);
			ImGui::Text("Max memory allocation count %d", properties.limits.maxMemoryAllocationCount);
			ImGui::Text("Max per stage resources %d", properties.limits.maxPerStageResources);
			ImGui::Text("Max push constants size %d", properties.limits.maxPushConstantsSize);
			ImGui::Text("Max sample mask words %d", properties.limits.maxSampleMaskWords);
			ImGui::Text("Max sampler allocation count %d", properties.limits.maxSamplerAllocationCount);
			ImGui::Text("Max sampler anisotropy %f", properties.limits.maxSamplerAnisotropy);
			ImGui::Text("Max sampler LOD bias %f", properties.limits.maxSamplerLodBias);
			ImGui::Text("Max storage buffer range %d", properties.limits.maxStorageBufferRange);
			ImGui::Text("Max texelbuffer elements %d", properties.limits.maxTexelBufferElements);
			ImGui::Text("Max texel gather offset %d", properties.limits.maxTexelGatherOffset);
			ImGui::Text("Max texel offset %d", properties.limits.maxTexelOffset);
			ImGui::Text("Max uniform buffer range %d", properties.limits.maxUniformBufferRange);
			ImGui::Text("Max viewport dimensions %d, %d", properties.limits.maxViewportDimensions[0], properties.limits.maxViewportDimensions[1]);
			ImGui::Text("Max viewports %d", properties.limits.maxViewports);
			ImGui::Text("Min interpolation offset %f", properties.limits.minInterpolationOffset);
			ImGui::Text("Min memory map alignment %d", (int)properties.limits.minMemoryMapAlignment);
			ImGui::Text("Min storage buffer offset alignment %lu", properties.limits.minStorageBufferOffsetAlignment);
			ImGui::Text("Min texel buffer offset alignment %lu", properties.limits.minTexelBufferOffsetAlignment);
			ImGui::Text("Min texel gather offset %d", properties.limits.minTexelGatherOffset);
			ImGui::Text("Min texel offset %d", properties.limits.minTexelOffset);
			ImGui::Text("Min uniform buffer offset alignment %lu", properties.limits.minUniformBufferOffsetAlignment);
			ImGui::Text("Mipmap precision bits %d", properties.limits.mipmapPrecisionBits);
			ImGui::Text("Non coherent atom size %lu", properties.limits.nonCoherentAtomSize);
			ImGui::Text("Optimal buffer copy offset alignment %lu", properties.limits.optimalBufferCopyOffsetAlignment);
			ImGui::Text("Optimal buffer copy row pitch alignment %lu", properties.limits.optimalBufferCopyRowPitchAlignment);
			ImGui::Text("Point size granularoty %f", properties.limits.pointSizeGranularity);
			ImGui::Text("Point size range %f, %f", properties.limits.pointSizeRange[0], properties.limits.pointSizeRange[1]);
			ImGui::Text("Sampled image color saple counts %d", properties.limits.sampledImageColorSampleCounts);
			ImGui::Text("Sampled image depth sample counts %d", properties.limits.sampledImageDepthSampleCounts);
			ImGui::Text("Sampled image integer sample counts %d", properties.limits.sampledImageIntegerSampleCounts);
			ImGui::Text("Sampled image stencil sample counts %d", properties.limits.sampledImageStencilSampleCounts);
			ImGui::Text("Sparse address space size %lu", properties.limits.sparseAddressSpaceSize);
			ImGui::Text("Standard sample locations %d", properties.limits.standardSampleLocations);
			ImGui::Text("Storage image sample counts %d", properties.limits.storageImageSampleCounts);
			ImGui::Text("Strict lines %d", properties.limits.strictLines);
			ImGui::Text("Sub pixel interpolation offset bits %d", properties.limits.subPixelInterpolationOffsetBits);
			ImGui::Text("Sub pixel precision bits %d", properties.limits.subPixelPrecisionBits);
			ImGui::Text("Sub texel precision bits %d", properties.limits.subTexelPrecisionBits);
			ImGui::Text("Timestamp compute and graphics %d", properties.limits.timestampComputeAndGraphics);
			ImGui::Text("Timestamp period %f", properties.limits.timestampPeriod);
			ImGui::Text("Viewport bounds range %f, %f", properties.limits.viewportBoundsRange[0], properties.limits.viewportBoundsRange[1]);
			ImGui::Text("Viewport subpixel bits %d", properties.limits.viewportSubPixelBits);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Features"))
		{
			ImGui::Text("Alpha to one %s", features.alphaToOne ? "TRUE" : "FALSE");
			ImGui::Text("Depth bias clamp %s", features.depthBiasClamp ? "TRUE" : "FALSE");
			ImGui::Text("Depth bounds %s", features.depthBounds ? "TRUE" : "FALSE");
			ImGui::Text("Depth clamp %s", features.depthClamp ? "TRUE" : "FALSE");
			ImGui::Text("Draw indirect first instance %s", features.drawIndirectFirstInstance ? "TRUE" : "FALSE");
			ImGui::Text("Dual src blend %s", features.dualSrcBlend ? "TRUE" : "FALSE");
			ImGui::Text("Fill mode non solid %s", features.fillModeNonSolid ? "TRUE" : "FALSE");
			ImGui::Text("Fragment stores and atomics %s", features.fragmentStoresAndAtomics ? "TRUE" : "FALSE");
			ImGui::Text("Full draw index unit32 %s", features.fullDrawIndexUint32 ? "TRUE" : "FALSE");
			ImGui::Text("Geometry shader %s", features.geometryShader ? "TRUE" : "FALSE");
			ImGui::Text("Image cube array %s", features.imageCubeArray ? "TRUE" : "FALSE");
			ImGui::Text("Independent blend %s", features.independentBlend ? "TRUE" : "FALSE");
			ImGui::Text("Inherited queries %s", features.inheritedQueries ? "TRUE" : "FALSE");
			ImGui::Text("Large points %s", features.largePoints ? "TRUE" : "FALSE");
			ImGui::Text("Logic op %s", features.logicOp ? "TRUE" : "FALSE");
			ImGui::Text("Multi draw indirect %s", features.multiDrawIndirect ? "TRUE" : "FALSE");
			ImGui::Text("Multi viewport %s", features.multiViewport ? "TRUE" : "FALSE");
			ImGui::Text("Occlusion query precise %s", features.occlusionQueryPrecise ? "TRUE" : "FALSE");
			ImGui::Text("Pipeline statistics query %s", features.pipelineStatisticsQuery ? "TRUE" : "FALSE");
			ImGui::Text("Robest buffer access %s", features.robustBufferAccess ? "TRUE" : "FALSE");
			ImGui::Text("Sampler anisotropy %s", features.samplerAnisotropy ? "TRUE" : "FALSE");
			ImGui::Text("Sample rate shading %s", features.sampleRateShading ? "TRUE" : "FALSE");
			ImGui::Text("Shader clip distance %s", features.shaderClipDistance ? "TRUE" : "FALSE");
			ImGui::Text("Shader cull distance %s", features.shaderClipDistance ? "TRUE" : "FALSE");
			ImGui::Text("Shader float64 %s", features.shaderFloat64 ? "TRUE" : "FALSE");
			ImGui::Text("Shader image gather extended %s", features.shaderImageGatherExtended ? "TRUE" : "FALSE");
			ImGui::Text("Shader int 16 %s", features.shaderInt16 ? "TRUE" : "FALSE");
			ImGui::Text("Shader int 64 %s", features.shaderInt64 ? "TRUE" : "FALSE");
			ImGui::Text("Shader resource min LOD %s", features.shaderResourceMinLod ? "TRUE" : "FALSE");
			ImGui::Text("Shader resource residency %s", features.shaderResourceResidency ? "TRUE" : "FALSE");
			ImGui::Text("Shader sampled image array dynamic indexing %s", features.shaderSampledImageArrayDynamicIndexing ? "TRUE" : "FALSE");
			ImGui::Text("Shader storage buffer array dynamic indexing %s", features.shaderStorageBufferArrayDynamicIndexing ? "TRUE" : "FALSE");
			ImGui::Text("Shader storage image arrya dynamic indexing %s", features.shaderStorageImageArrayDynamicIndexing ? "TRUE" : "FALSE");
			ImGui::Text("Shader storage image extended formats %s", features.shaderStorageImageExtendedFormats ? "TRUE" : "FALSE");
			ImGui::Text("Shader storage image multisample %s", features.shaderStorageImageMultisample ? "TRUE" : "FALSE");
			ImGui::Text("Shader storage image read without format %s", features.shaderStorageImageReadWithoutFormat ? "TRUE" : "FALSE");
			ImGui::Text("Shader storage image write without format %s", features.shaderStorageImageWriteWithoutFormat ? "TRUE" : "FALSE");
			ImGui::Text("Shader tessellation and geometry point size %s", features.shaderTessellationAndGeometryPointSize ? "TRUE" : "FALSE");
			ImGui::Text("Shader uniform buffer array dynamic indexing %s", features.shaderUniformBufferArrayDynamicIndexing ? "TRUE" : "FALSE");
			ImGui::Text("Sparse binding %s", features.sparseBinding ? "TRUE" : "FALSE");
			ImGui::Text("Sparse residency 16 samples %s", features.sparseResidency16Samples ? "TRUE" : "FALSE");
			ImGui::Text("Sparse residency 2 samples %s", features.sparseResidency2Samples ? "TRUE" : "FALSE");
			ImGui::Text("Sparse residency 4 samples %s", features.sparseResidency4Samples ? "TRUE" : "FALSE");
			ImGui::Text("Sparse residency 8 samples %s", features.sparseResidency8Samples ? "TRUE" : "FALSE");
			ImGui::Text("Sparse residency aliased %s", features.sparseResidencyAliased ? "TRUE" : "FALSE");
			ImGui::Text("Sparse residency buffer %s", features.sparseResidencyBuffer ? "TRUE" : "FALSE");
			ImGui::Text("Sparse residency image 2D %s", features.sparseResidencyImage2D ? "TRUE" : "FALSE");
			ImGui::Text("Sparse residency image 3D %s", features.sparseResidencyImage3D ? "TRUE" : "FALSE");
			ImGui::Text("Tesselation shader %s", features.tessellationShader ? "TRUE" : "FALSE");
			ImGui::Text("Texture compression ASTC LDR %s", features.textureCompressionASTC_LDR ? "TRUE" : "FALSE");
			ImGui::Text("Texture compression BC %s", features.textureCompressionBC ? "TRUE" : "FALSE");
			ImGui::Text("Texture compression ETC2 %s", features.textureCompressionETC2 ? "TRUE" : "FALSE");
			ImGui::Text("Variable multisample rate %s", features.variableMultisampleRate ? "TRUE" : "FALSE");
			ImGui::Text("Vertex pipeline stores and atomics %s", features.vertexPipelineStoresAndAtomics ? "TRUE" : "FALSE");
			ImGui::Text("Wide lines %s", features.wideLines ? "TRUE" : "FALSE");
			
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Memory"))
		{
			if (ImGui::TreeNode("Types"))
			{
				for(int i=0 ; i<memoryProperties.memoryTypeCount ; ++i)
				{
					char buffer[64];
					sprintf(&buffer[0], "Heap %d", memoryProperties.memoryTypes[i].heapIndex);
					if (ImGui::TreeNode(buffer))
					{
						if(memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) { ImGui::Text("Device coherent AMD"); }
						if(memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) { ImGui::Text("Device local"); }
						if(memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD) { ImGui::Text("Device uncached AMD"); }
						if(memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) { ImGui::Text("Host cached"); }
						if(memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) { ImGui::Text("Host coherent"); }
						if(memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) { ImGui::Text("Host visible"); }
						if(memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) { ImGui::Text("Lazily allocated"); }
						if(memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) { ImGui::Text("Protected"); }
						if(memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV) { ImGui::Text("RDMA capable"); }
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Heaps"))
			{
				for(int i=0 ; i<memoryProperties.memoryHeapCount ; ++i)
				{
					char buffer[64];
					sprintf(&buffer[0], "Heap %d", i);
					if (ImGui::TreeNode(buffer))
					{
						ImGui::Text("Size %llu MB", memoryProperties.memoryHeaps[i].size / (1024 * 1024));
						if(memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) { ImGui::Text("Device local"); }
						if(memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) { ImGui::Text("Multi instance"); }
						if(memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR) { ImGui::Text("Multi instance KHR"); }
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}
			
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Queues"))
		{
			int index = 0;
			for(auto queue : queueFamilyProperties)
			{
				char buffer[64];
				sprintf(&buffer[0], "Queue %d", index);
				if (ImGui::TreeNode(buffer))
				{
					if(queue.queueFlags & VK_QUEUE_COMPUTE_BIT){ ImGui::Text("Compute");}
					if(queue.queueFlags & VK_QUEUE_GRAPHICS_BIT){ ImGui::Text("Graphics");}
					if(queue.queueFlags & VK_QUEUE_PROTECTED_BIT){ ImGui::Text("Protected");}
					if(queue.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT){ ImGui::Text("Sparse binding");}
					if(queue.queueFlags & VK_QUEUE_TRANSFER_BIT){ ImGui::Text("Transfer");}
					ImGui::Text("Count %d", queue.queueCount);
					ImGui::Text("Timestamp valid %d", queue.timestampValidBits);
					ImGui::Text("Image transfer granularity width %d", queue.minImageTransferGranularity.width );
					ImGui::Text("Image transfer granularity height %d", queue.minImageTransferGranularity.height );
					ImGui::Text("Image transfer granularity depth %d", queue.minImageTransferGranularity.depth );
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
}

void VulkanPhysicalDevice::LogDeviceName()
{
	LOGINFOF("Vulkan physical device: %s", properties.deviceName);
}

void VulkanPhysicalDevice::LogDeviceInfo()
{
    LOGINFO("");
	LOGINFO("=== Vulkan Physical Device Info ===");
	LOGINFO("");
	LOGINFO("--- Properties ---");
	LOGINFO("");
	// Properties
	LOGINFOF("Name: %s", properties.deviceName);
    LOGINFOF("API version: %u", properties.apiVersion);
    LOGINFOF("Driver version: %u", properties.driverVersion);
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

bool VulkanPhysicalDevice::IsDiscreetGPU()
{
	return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}
