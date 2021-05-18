@PACKAGE_INIT@

if(NOT TARGET @PROJECT_NAME@)
	find_package(Threads)
	include("${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@Targets.cmake")
endif()

check_required_components(@PROJECT_NAME@)