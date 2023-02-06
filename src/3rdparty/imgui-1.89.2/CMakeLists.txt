set(SOURCE_FILES ${SOURCE_FILES}
	${CMAKE_CURRENT_LIST_DIR}/imgui_demo.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui_draw.cpp
	${CMAKE_CURRENT_LIST_DIR}/backends/imgui_impl_sdl.cpp
	${CMAKE_CURRENT_LIST_DIR}/backends/imgui_impl_vulkan.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui_tables.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui_widgets.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui.cpp
	PARENT_SCOPE)