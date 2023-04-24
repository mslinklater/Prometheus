// [+Header]
// [-Header]

#include "vulkanhandlemanager.h"
#include <map>

static std::map<void*, std::string> handleMap;

void VulkanHandleManager::AddHandle(void* handle, const std::string& name)
{
	handleMap[handle] = name;
}

const std::string& VulkanHandleManager::GetName(void* handle)
{
	return handleMap[handle];
}

void VulkanHandleManager::RemoveHandle(void* handle)
{
	handleMap.erase(handle);
}
