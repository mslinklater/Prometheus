// [+Header]
// [-Header]

#include "vulkanhandlemanager.h"
#include <map>

static std::map<void*, std::string> handleMap;

void VulkanHandleManager_Old::AddHandle(void* handle, const std::string& name)
{
	handleMap[handle] = name;
}

const std::string& VulkanHandleManager_Old::GetName(void* handle)
{
	return handleMap[handle];
}

void VulkanHandleManager_Old::RemoveHandle(void* handle)
{
	handleMap.erase(handle);
}
