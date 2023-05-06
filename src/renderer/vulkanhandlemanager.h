// [+Header]
// [-Header]

// this is mainly used for debugging - should be defined out in release builds

#pragma once
#include <string>
#include <map>

namespace VulkanHandleManager_Old
{
	void AddHandle(void* handle, const std::string& name);
	const std::string& GetName(void* handle);
	void RemoveHandle(void* handle);
}

#if 0
template<typename T>
class VulkanHandleManager
{
public:
	static void AddHandle(T handle, const std::string& name)
	{
		handleMap[handle] = name;
	}
	static const std::string& GetName(T handle)
	{
		return handleMap[handle];
	}
	static void RemoveHandle(T handle)
	{
		handleMap.erase(handle);
	}
private:
	static std::map<T, std::string> handleMap;
};
#endif