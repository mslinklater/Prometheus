#include "errors.h"
#include "log.h"

#define LOGANDRETURNLINE(x)                                                    \
    case x:                                                                    \
        LOGERROR(#x);                                                          \
        break

EError ErrorLogAndReturn(EError error)
{
    switch (error)
    {
    case EError::OK:
        LOGINFO("OK");
        break;

        // SDL
        LOGANDRETURNLINE(EError::SDL_UnableToInitialize);
        LOGANDRETURNLINE(EError::SDL_CouldNotCreateWindow);
        LOGANDRETURNLINE(EError::SDL_CouldNotGetRequiredVulkanExtensions);
        LOGANDRETURNLINE(EError::SDL_CouldNotCreateVulkanSurface);

        // Vulkan
        LOGANDRETURNLINE(EError::Vulkan_UnableToFindDriver);
        LOGANDRETURNLINE(EError::Vulkan_CouldNotCreateInstance);
        LOGANDRETURNLINE(EError::Vulkan_CouldNotCreateSurface);
        LOGANDRETURNLINE(EError::Vulkan_CouldNotEnumeratePhysicalDevices);

    // default
    default:
        LOGERROR("Unknown");
        break;
    }
    return error;
}