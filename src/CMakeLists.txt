cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(Prometheus LANGUAGES CXX)

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

set(CMAKE_C_FLAGS "-DVK_ENABLE_BETA_EXTENSIONS -D_DEBUG")
set(CMAKE_CXX_FLAGS "-DVK_ENABLE_BETA_EXTENSIONS -D_DEBUG")

set(SOURCE_FILES main.cpp)
set(IMGUI_PATH "3rdparty/imgui-1.89.2")

add_subdirectory(${IMGUI_PATH})
add_subdirectory("system")
add_subdirectory("renderer")

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${Vulkan_INCLUDE_DIRS})
include_directories(${IMGUI_PATH})
include_directories(".")

add_executable(prometheus ${SOURCE_FILES})
target_link_libraries(prometheus ${SDL2_LIBRARIES} ${Vulkan_LIBRARIES})
