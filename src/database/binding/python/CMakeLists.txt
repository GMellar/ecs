
find_package(PythonLibs 3)
find_package(Boost system filesystem python3 numpy3)

if(${PYTHONLIBS_FOUND} AND ${Boost_FOUND})
	add_library(ecspy MODULE 
		"${CMAKE_CURRENT_SOURCE_DIR}/ConnectionParameters.cpp")
	target_link_libraries(ecspy ecs ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})
	target_include_directories(ecspy PRIVATE ${PYTHON_INCLUDE_DIRS})
	set_target_properties(ecspy PROPERTIES PREFIX "")
endif()
