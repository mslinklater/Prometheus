cmake_minimum_required(VERSION 3.10)

project(faffaboutengine)

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${Vulkan_INCLUDE_DIRS})

if(${CMAKE_BUILD_TYPE} EQUAL Debug)
	set(CMAKE_C_FLAGS "-DDEBUG")
endif()

set(SOURCE_FILES main.cpp errors.cpp log.cpp config.cpp)

add_subdirectory("renderer")
add_subdirectory("supereasyjson")

add_executable(faffaboutengine ${SOURCE_FILES})
target_link_libraries(faffaboutengine ${SDL2_LIBRARIES} ${Vulkan_LIBRARIES})
target_include_directories(faffaboutengine PRIVATE ".")