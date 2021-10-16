#pragma once

#include <string>

enum EError
{
	OK = 0,

	// SDL

	SDL_UnableToInitialize,
	SDL_CouldNotCreateWindow,
	SDL_CouldNotGetRequiredVulkanExtensions,

	// Vulkan

	Vulkan_UnableToFindDriver,
	Vulkan_CouldNotCreateInstance,
	Vulkan_CouldNotCreateSurface,
	Vulkan_CouldNotEnumeratePhysicalDevices,

	Unknown,
};

void Error_Log(std::string errorString);
