cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(Prometheus LANGUAGES CXX)

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${Vulkan_INCLUDE_DIRS})

add_executable(prometheus main.cpp imgui_impl_sdl.cpp imgui_impl_vulkan.cpp imgui_demo.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp imgui.cpp)
target_link_libraries(prometheus ${SDL2_LIBRARIES} ${Vulkan_LIBRARIES})
