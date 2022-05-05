set(SOURCE_FILES ${SOURCE_FILES}
	${CMAKE_CURRENT_LIST_DIR}/config.cpp
	${CMAKE_CURRENT_LIST_DIR}/log.cpp
	${CMAKE_CURRENT_LIST_DIR}/errors.cpp
	PARENT_SCOPE
)

add_subdirectory("lua")
