include(CMakeFindDependencyMacro)

# Don't include when used as package inside the project tree
if(NOT TARGET ecs)
	include("${CMAKE_CURRENT_LIST_DIR}/EcsTargets.cmake")
endif()
