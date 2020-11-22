include(CMakePackageConfigHelpers)

# Include catch test framework
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/catch2")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ecscore")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ecsconfig")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ecsdynlib")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ecstools")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ecsnet")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ecsdb")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ecsdocstore")
#add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/apps")

# All testing applications 
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/testing")

# To be able to use the libraries from this project inside other projects we 
# need to deploy them. We use cmake configuration files for this. 
configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/ecs-config.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/ecs-config.cmake"
	INSTALL_DESTINATION "${INSTALL_CMAKE_CONFIG}/ecs")
install(FILES 
	"${CMAKE_CURRENT_BINARY_DIR}/ecs-config.cmake"
	DESTINATION "${INSTALL_CMAKE_CONFIG}/ecs")
