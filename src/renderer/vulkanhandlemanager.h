// [+Header]
// [-Header]

#pragma once
#include <string>

namespace VulkanHandleManager
{
	void AddHandle(void* handle, const std::string& name);
	const std::string& GetName(void* handle);
	void RemoveHandle(void* handle);
}
