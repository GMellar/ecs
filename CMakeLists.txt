cmake_minimum_required(VERSION 3.10)
project(ecs LANGUAGES C CXX VERSION 0.4.1)

if(TARGET ecs) 
	return()
endif()

enable_testing()

option(BUILD_OFFLINE "Build offline" ON)

include(ExternalProject)
include(FetchContent)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_CXX_STANDARD)
	set(CMAKE_CXX_STANDARD 17)
endif()

find_library(DL_LIBRARY NAMES dl)
find_package(Threads)
find_package(Boost REQUIRED COMPONENTS system filesystem) 
find_package(Git REQUIRED)


FetchContent_Declare(Catch2
  GIT_REPOSITORY    "https://github.com/catchorg/Catch2"
  GIT_TAG "v2.13.6"
  SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/external/catch2"
)
FetchContent_GetProperties(Catch2)
if(NOT Catch2_POPULATED)
  FetchContent_Populate(Catch2)
  add_subdirectory("${CMAKE_CURRENT_BINARY_DIR}/external/catch2")
endif()

file(GLOB ecs_sources_base 
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/impl/LibraryImplBase.cpp")

file(GLOB ecs_database_base 
	"${CMAKE_CURRENT_SOURCE_DIR}/src/database/*.cpp")

add_library(${PROJECT_NAME}_obj OBJECT 
		${ecs_sources_base}
		${ecs_database_base}
)

target_include_directories(${PROJECT_NAME}_obj
	PRIVATE 
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
		"${CMAKE_CURRENT_BINARY_DIR}/include"
		${Boost_INCLUDE_DIRS}
	INTERFACE 
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
		$<BUILD_INTERFACE:${Boost_INCLUDE_DIRS}>
		$<INSTALL_INTERFACE:${Boost_INCLUDE_DIRS}>
)

target_link_libraries(${PROJECT_NAME}_obj
	PRIVATE 
		Threads::Threads
		postgresql_dbplugin_obj
		sqlite3_dbplugin_obj
	INTERFACE 
		Threads::Threads
		${Boost_LIBRARIES}
)

if(WIN32) 
	target_compile_definitions(${PROJECT_NAME}_obj 
		PUBLIC ECS_PLATFORM_WINDOWS
	)
endif()

if(DL_LIBRARY) 
	target_link_libraries(${PROJECT_NAME}_obj 
		PUBLIC 
			${DL_LIBRARY})
endif()

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/database/sqlite3")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/database/postgresql")
	
add_library(${PROJECT_NAME} STATIC)
target_link_libraries(${PROJECT_NAME} 
	PUBLIC
		postgresql_dbplugin_obj
		sqlite3_dbplugin_obj
		${PROJECT_NAME}_obj
)
target_include_directories(ecs 
	PUBLIC
		$<INSTALL_INTERFACE:"${CMAKE_INSTALL_INCLUDEDIR}/ecs">
	INTERFACE 
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
		$<INSTALL_INTERFACE:"${CMAKE_INSTALL_INCLUDEDIR}/ecs">
)		
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/config.hpp.cmake" "${CMAKE_CURRENT_BINARY_DIR}/include/ecs/config.hpp")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/ecs/config.hpp" 
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/ecs")
generate_export_header(${PROJECT_NAME} 
	EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/include/ecs/ecs_export.h")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/ecs/ecs_export.h" 
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/ecs")

# Header file installation
file(GLOB public_headers "${CMAKE_SOURCE_DIR}/include/ecs/*.hpp")
file(GLOB public_database_headers "${CMAKE_SOURCE_DIR}/include/ecs/database/*.hpp")
install(FILES ${public_headers} 
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/ecs")
install(FILES ${public_database_headers} 
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/ecs/database")	

add_executable(${PROJECT_NAME}_test "${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp")
add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
target_link_libraries(${PROJECT_NAME}_test 
	PUBLIC 
		${PROJECT_NAME} Catch2::Catch2)

install(TARGETS ${PROJECT_NAME} ecs_obj postgresql_dbplugin_obj sqlite3_dbplugin_obj
	EXPORT "${PROJECT_NAME}Targets"
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
)
install(EXPORT "${PROJECT_NAME}Targets"
	FILE "${PROJECT_NAME}Targets.cmake"
	NAMESPACE ecs::
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)	

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/PackageConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake")
configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/PackageConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

#export(EXPORT ${PROJECT_NAME}Targets
#	FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake"
#	NAMESPACE ecs::
#)

install(FILES 
	"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")


set(CPACK_PACKAGE_CONTACT "Geoffrey Mellar <mellar@gamma-kappa.com>")
set(CPACK_GENERATOR "ZIP")

# Check for debian flavor
find_program(LSB_RELEASE_EXEC lsb_release)
if(LSB_RELEASE_EXEC)
	execute_process(COMMAND ${LSB_RELEASE_EXEC} -is
	    OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT
	    OUTPUT_STRIP_TRAILING_WHITESPACE
	)
endif()

if(LSB_RELEASE_ID_SHORT)
	set(CPACK_GENERATOR "${CPACK_GENERATOR};DEB")
endif()

include(CPack)
