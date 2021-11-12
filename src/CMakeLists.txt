cmake_minimum_required(VERSION 3.10)

project(prometheus)

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${Vulkan_INCLUDE_DIRS})

set(SOURCE_FILES main.cpp)

add_subdirectory("system")
add_subdirectory("renderer")
add_subdirectory("physicsengine")
add_subdirectory("supereasyjson")

add_executable(prometheus ${SOURCE_FILES})
target_link_libraries(prometheus ${SDL2_LIBRARIES} ${Vulkan_LIBRARIES})
target_include_directories(prometheus PRIVATE ".")
