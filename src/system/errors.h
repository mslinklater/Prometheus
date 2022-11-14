#pragma once

#include <string>

enum EError
{
    OK = 0,

    // SDL

    SDL_UnableToInitialize,
    SDL_CouldNotCreateWindow,
    SDL_CouldNotGetRequiredVulkanExtensions,
    SDL_CouldNotCreateVulkanSurface,

    // Vulkan

    Vulkan_UnableToFindDriver,
    Vulkan_ExtensionNotPresent,
    Vulkan_CouldNotCreateInstance,
    Vulkan_CouldNotCreateSurface,
    Vulkan_CouldNotEnumeratePhysicalDevices,

    Unknown,
};

EError ErrorLogAndReturn(EError error);
